//! Task thread wrapper that owns a background run loop and exposes its handle.

#![allow(dead_code)]

use crate::run_loop::{RunLoop, RunLoopError, RunLoopHandle};
use std::thread::{self, JoinHandle};

/// Background task thread with an internal run loop.
pub struct TaskThread {
    handle: RunLoopHandle,
    join_handle: Option<JoinHandle<()>>,
}

impl TaskThread {
    /// Start a new task thread and run loop.
    pub fn start() -> Self {
        Self::start_with_init(|_| {})
    }

    /// Start a new task thread and execute custom setup code before `run_loop.run()`.
    /// The callback runs on the task thread and receives a cloneable handle that can be
    /// used to post follow-up work.
    pub fn start_with_init<F>(before_run: F) -> Self
    where
        F: FnOnce(RunLoopHandle) + Send + 'static,
    {
        let (run_loop, handle) = RunLoop::new();
        let init_handle = handle.clone();
        let join_handle = thread::Builder::new()
            .name("boyia-task-thread".to_string())
            .spawn(move || {
                before_run(init_handle);
                run_loop.run();
            })
            .expect("failed to spawn task thread");
        Self {
            handle,
            join_handle: Some(join_handle),
        }
    }

    /// Get a cloneable run-loop handle for posting tasks.
    pub fn handle(&self) -> RunLoopHandle {
        self.handle.clone()
    }

    /// Post a task into the task thread.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce() + Send + 'static,
    {
        self.handle.post_task(task)
    }

    /// Request the task thread to stop.
    pub fn stop(&self) -> Result<(), RunLoopError> {
        self.handle.stop()
    }

    /// Stop the task thread and wait for it to finish.
    pub fn join(mut self) -> std::thread::Result<()> {
        let _ = self.handle.stop();
        if let Some(join_handle) = self.join_handle.take() {
            join_handle.join()
        } else {
            Ok(())
        }
    }
}
