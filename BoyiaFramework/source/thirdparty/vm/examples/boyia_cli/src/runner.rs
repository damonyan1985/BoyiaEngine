//! Runner wrapper that binds BoyiaRuntime to a dedicated TaskThread.
//! All runtime operations are posted onto the task thread so BoyiaRuntime
//! stays on a single thread for its whole lifetime.

#![allow(dead_code)]

use crate::https::builtin_https_class;
use crate::run_loop::{RunLoopError, RunLoopHandle};
use crate::task_thread::TaskThread;
use crate::thread_pool::ThreadPool;
use boyia_runtime::BoyiaRuntime;
use std::sync::{mpsc, Arc};

const DEFAULT_HTTPS_THREAD_COUNT: usize = 4;

pub struct BoyiaRunner {
    task_thread: Option<TaskThread<Box<BoyiaRuntime>>>,
    thread_pool: Option<Arc<ThreadPool>>,
    ready: bool,
}

impl BoyiaRunner {
    /// Create a runner and initialize BoyiaRuntime on the task thread before the run loop starts.
    /// Returns a box so the Https class can hold a raw pointer to the same runner.
    pub fn create() -> Box<Self> {
        let (ready_tx, ready_rx) = mpsc::channel();

        let task_thread = TaskThread::start_with_init(move |_| {
            let runtime = BoyiaRuntime::create();
            let ready = !runtime.vm().is_null();
            let _ = ready_tx.send(ready);
            runtime
        });

        let ready = ready_rx.recv().unwrap_or(false);
        let thread_pool = Arc::new(ThreadPool::new(DEFAULT_HTTPS_THREAD_COUNT));

        let runner = Self {
            task_thread: Some(task_thread),
            thread_pool: Some(thread_pool),
            ready,
        };
        let runner_box = Box::new(runner);
        let runner_ptr_usize = runner_box.as_ref() as *const BoyiaRunner as usize;

        let (init_tx, init_rx) = mpsc::channel();
        let _ = runner_box
            .task_thread
            .as_ref()
            .unwrap()
            .post_task(move |runtime| {
                let vm = runtime.vm();
                let mut gen_id = |s: &str| runtime.id_creator().gen_ident_by_str(s);
                let runner_ptr = runner_ptr_usize as *mut BoyiaRunner;
                builtin_https_class(vm, &mut gen_id, runner_ptr);
                let _ = init_tx.send(());
            });
        let _ = init_rx.recv();

        runner_box
    }

    /// Get handle and thread pool from a runner pointer (for Https scheduling). Returns None if pointer is null or runner is not fully initialized.
    pub unsafe fn get_handle_and_pool_from_ptr(
        runner: *mut BoyiaRunner,
    ) -> Option<(RunLoopHandle<Box<BoyiaRuntime>>, std::sync::Weak<ThreadPool>)> {
        if runner.is_null() {
            return None;
        }
        (*runner).handle_and_pool()
    }

    fn handle_and_pool(
        &self,
    ) -> Option<(RunLoopHandle<Box<BoyiaRuntime>>, std::sync::Weak<ThreadPool>)> {
        let handle = self.task_thread.as_ref()?.handle();
        let pool = Arc::downgrade(self.thread_pool.as_ref()?);
        Some((handle, pool))
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
        if let Some(thread_pool) = self.thread_pool.take() {
            let _ = thread_pool.stop();
            if let Ok(thread_pool) = Arc::try_unwrap(thread_pool) {
                let _ = thread_pool.join();
            }
        }
        if let Some(task_thread) = self.task_thread.take() {
            let _ = task_thread.join();
        }
    }
}
