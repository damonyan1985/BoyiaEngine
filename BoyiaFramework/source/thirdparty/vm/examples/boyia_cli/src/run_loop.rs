//! Simple task queue run loop for examples.
//! Tasks are executed serially in posting order.

#![allow(dead_code)]

use std::sync::mpsc::{self, Receiver, Sender};

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
}

/// Handle used by other threads to post tasks into the run loop.
pub struct RunLoopHandle<T> {
    sender: Sender<RunLoopMessage<T>>,
}

impl<T> Clone for RunLoopHandle<T> {
    fn clone(&self) -> Self {
        Self {
            sender: self.sender.clone(),
        }
    }
}

impl<T> RunLoop<T> {
    /// Create a run loop and its posting handle.
    pub fn new() -> (Self, RunLoopHandle<T>) {
        let (sender, receiver) = mpsc::channel();
        (
            Self { receiver },
            RunLoopHandle { sender },
        )
    }

    /// Block the current thread and process tasks until `stop()` is posted.
    pub fn run(self, mut context: T) {
        while let Ok(message) = self.receiver.recv() {
            match message {
                RunLoopMessage::Task(task) => task(&mut context),
                RunLoopMessage::Stop => break,
            }
        }
    }
}

impl<T> RunLoopHandle<T> {
    /// Post a task into the run loop with mutable access to the thread-owned context.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce(&mut T) + Send + 'static,
    {
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
