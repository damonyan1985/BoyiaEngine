//! Runner wrapper that binds BoyiaRuntime to a dedicated TaskThread.
//! All runtime operations are posted onto the task thread so BoyiaRuntime
//! stays on a single thread for its whole lifetime.

#![allow(dead_code)]

use crate::run_loop::RunLoopError;
use crate::task_thread::TaskThread;
use boyia_runtime::BoyiaRuntime;
use std::cell::RefCell;
use std::sync::mpsc;

thread_local! {
    static TASK_THREAD_RUNTIME: RefCell<Option<Box<BoyiaRuntime>>> = const { RefCell::new(None) };
}

pub struct BoyiaRunner {
    task_thread: Option<TaskThread>,
    ready: bool,
}

impl BoyiaRunner {
    /// Create a runner and initialize BoyiaRuntime on the task thread before the run loop starts.
    pub fn create() -> Self {
        let (ready_tx, ready_rx) = mpsc::channel();

        let task_thread = TaskThread::start_with_init(move |_| {
            TASK_THREAD_RUNTIME.with(|runtime| {
                let runtime_instance = BoyiaRuntime::create();
                let ready = !runtime_instance.vm().is_null();
                *runtime.borrow_mut() = Some(runtime_instance);
                let _ = ready_tx.send(ready);
            });
        });

        let ready = ready_rx.recv().unwrap_or(false);

        Self {
            task_thread: Some(task_thread),
            ready,
        }
    }

    pub fn is_ready(&self) -> bool {
        self.ready
    }

    fn post_runtime_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce(&mut BoyiaRuntime) + Send + 'static,
    {
        self.task_thread
            .as_ref()
            .expect("task thread already taken")
            .post_task(move || {
                TASK_THREAD_RUNTIME.with(|runtime| {
                    let mut runtime = runtime.borrow_mut();
                    if let Some(runtime) = runtime.as_deref_mut() {
                        task(runtime);
                    }
                });
            })
    }

    pub fn compile(&self, script: &str) -> Result<(), RunLoopError> {
        let script = script.to_string();
        let (done_tx, done_rx) = mpsc::channel();
        self.post_runtime_task(move |runtime| {
            runtime.compile(&script);
            let _ = done_tx.send(());
        })?;
        let _ = done_rx.recv();
        Ok(())
    }

    pub fn run_exe_file(&self) -> Result<(), RunLoopError> {
        let (done_tx, done_rx) = mpsc::channel();
        self.post_runtime_task(move |runtime| {
            runtime.run_exe_file();
            let _ = done_tx.send(());
        })?;
        let _ = done_rx.recv();
        Ok(())
    }

    pub fn consume_micro_task(&self) -> Result<(), RunLoopError> {
        let (done_tx, done_rx) = mpsc::channel();
        self.post_runtime_task(move |runtime| {
            runtime.consume_micro_task();
            let _ = done_tx.send(());
        })?;
        let _ = done_rx.recv();
        Ok(())
    }
}

impl Drop for BoyiaRunner {
    fn drop(&mut self) {
        if let Some(task_thread) = self.task_thread.take() {
            let _ = task_thread.post_task(move || {
                TASK_THREAD_RUNTIME.with(|runtime| {
                    let _ = runtime.borrow_mut().take();
                });
            });
            let _ = task_thread.join();
        }
    }
}
