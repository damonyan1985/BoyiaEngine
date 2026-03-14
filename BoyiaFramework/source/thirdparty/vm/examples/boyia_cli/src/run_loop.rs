//! Simple task queue run loop for examples.
//! Tasks are executed serially in posting order.

#![allow(dead_code)]

use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::Arc;

type Task<T> = Box<dyn FnOnce(&mut T) + Send + 'static>;

enum RunLoopMessage<T> {
    Task(Task<T>),
    Stop,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RunLoopError {
    Stopped,
}

/// Single-consumer run loop that processes posted tasks one by one.
pub struct RunLoop<T> {
    receiver: Receiver<RunLoopMessage<T>>,
    pending_count: Arc<AtomicUsize>,
}

/// Handle used by other threads to post tasks into the run loop.
pub struct RunLoopHandle<T> {
    sender: Sender<RunLoopMessage<T>>,
    pending_count: Arc<AtomicUsize>,
}

impl<T> Clone for RunLoopHandle<T> {
    fn clone(&self) -> Self {
        Self {
            sender: self.sender.clone(),
            pending_count: Arc::clone(&self.pending_count),
        }
    }
}

impl<T> RunLoop<T> {
    /// Create a run loop and its posting handle.
    pub fn new() -> (Self, RunLoopHandle<T>) {
        let (sender, receiver) = mpsc::channel();
        let pending_count = Arc::new(AtomicUsize::new(0));
        (
            Self {
                receiver,
                pending_count: Arc::clone(&pending_count),
            },
            RunLoopHandle {
                sender,
                pending_count,
            },
        )
    }

    /// Block the current thread and process tasks until `stop()` is posted.
    pub fn run(self, mut context: T) {
        while let Ok(message) = self.receiver.recv() {
            match message {
                RunLoopMessage::Task(task) => {
                    self.pending_count.fetch_sub(1, Ordering::Relaxed);
                    task(&mut context);
                }
                RunLoopMessage::Stop => break,
            }
        }
    }
}

impl<T> RunLoopHandle<T> {
    /// Returns true if there are tasks not yet processed by the run loop.
    pub fn has_pending_tasks(&self) -> bool {
        self.pending_count.load(Ordering::Relaxed) != 0
    }

    /// Post a task into the run loop with mutable access to the thread-owned context.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce(&mut T) + Send + 'static,
    {
        self.pending_count.fetch_add(1, Ordering::Relaxed);
        self.sender
            .send(RunLoopMessage::Task(Box::new(task)))
            .map_err(|_| RunLoopError::Stopped)
    }

    /// Stop the run loop after all previously posted tasks are handled.
    pub fn stop(&self) -> Result<(), RunLoopError> {
        self.sender
            .send(RunLoopMessage::Stop)
            .map_err(|_| RunLoopError::Stopped)
    }
}
