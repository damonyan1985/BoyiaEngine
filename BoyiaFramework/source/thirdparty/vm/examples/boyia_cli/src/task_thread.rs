//! Task thread wrapper that owns a background run loop and exposes its handle.

#![allow(dead_code)]

use crate::run_loop::{RunLoop, RunLoopError, RunLoopHandle};
use std::thread::{self, JoinHandle};

/// Background task thread with an internal run loop.
pub struct TaskThread<T> {
    handle: RunLoopHandle<T>,
    join_handle: Option<JoinHandle<()>>,
}

impl TaskThread<()> {
    /// Start a new task thread and run loop (OS thread name `boyia-task-thread`).
    pub fn start() -> Self {
        Self::start_with_name("boyia-task-thread")
    }

    /// Start with a custom OS thread name (see [`thread::Builder::name`]).
    pub fn start_with_name(thread_name: impl Into<String>) -> Self {
        Self::start_with_init(thread_name, |_| ())
    }
}

impl<T: 'static> TaskThread<T> {
    /// Start a new task thread and create its context before `run_loop.run()`.
    /// `thread_name` is passed to [`thread::Builder::name`] for debuggers / profilers.
    /// The callback runs on the task thread and receives a cloneable handle that can be
    /// used to post follow-up work after the context is initialized.
    pub fn start_with_init<F>(thread_name: impl Into<String>, before_run: F) -> Self
    where
        F: FnOnce(RunLoopHandle<T>) -> T + Send + 'static,
    {
        let (run_loop, handle) = RunLoop::new();
        let init_handle = handle.clone();
        let join_handle = thread::Builder::new()
            .name(thread_name.into())
            .spawn(move || {
                let context = before_run(init_handle);
                run_loop.run(context);
            })
            .expect("failed to spawn task thread");
        Self {
            handle,
            join_handle: Some(join_handle),
        }
    }

    /// Get a cloneable run-loop handle for posting tasks.
    pub fn handle(&self) -> RunLoopHandle<T> {
        self.handle.clone()
    }

    /// Number of tasks not yet completed (queued or currently running) on this thread.
    pub fn load(&self) -> usize {
        self.handle.load()
    }

    /// Post a task into the task thread with mutable access to its context.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce(&mut T) + Send + 'static,
    {
        self.handle.post_task(task)
    }

    /// Request the task thread to stop. Waits until the task queue is empty (all posted tasks executed), then sends Stop.
    pub fn stop(&self) -> Result<(), RunLoopError> {
        while self.handle.has_pending_tasks() {
            std::thread::yield_now();
        }
        self.handle.stop()
    }

    /// Wait for the task thread to finish (does not send stop; blocks until the thread exits on its own).
    pub fn join(mut self) -> std::thread::Result<()> {
        if let Some(join_handle) = self.join_handle.take() {
            join_handle.join()
        } else {
            Ok(())
        }
    }
}
