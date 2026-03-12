//! Runner wrapper that binds BoyiaRuntime to a dedicated TaskThread.
//! All runtime operations are posted onto the task thread so BoyiaRuntime
//! stays on a single thread for its whole lifetime.

#![allow(dead_code)]

use crate::run_loop::RunLoopError;
use crate::task_thread::TaskThread;
use boyia_runtime::BoyiaRuntime;
use std::sync::mpsc;

pub struct BoyiaRunner {
    task_thread: Option<TaskThread<Box<BoyiaRuntime>>>,
    ready: bool,
}

impl BoyiaRunner {
    /// Create a runner and initialize BoyiaRuntime on the task thread before the run loop starts.
    pub fn create() -> Self {
        let (ready_tx, ready_rx) = mpsc::channel();

        let task_thread = TaskThread::start_with_init(move |_| {
            let runtime = BoyiaRuntime::create();
            let ready = !runtime.vm().is_null();
            let _ = ready_tx.send(ready);
            runtime
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
            .post_task(move |runtime| task(runtime.as_mut()))
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
            let _ = task_thread.join();
        }
    }
}
