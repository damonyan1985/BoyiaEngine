//! Simple task queue run loop for examples.
//! Tasks are executed serially in posting order.

#![allow(dead_code)]

use std::sync::mpsc::{self, Receiver, Sender};

type Task = Box<dyn FnOnce() + Send + 'static>;

enum RunLoopMessage {
    Task(Task),
    Stop,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RunLoopError {
    Stopped,
}

/// Single-consumer run loop that processes posted tasks one by one.
pub struct RunLoop {
    receiver: Receiver<RunLoopMessage>,
}

/// Handle used by other threads to post tasks into the run loop.
#[derive(Clone)]
pub struct RunLoopHandle {
    sender: Sender<RunLoopMessage>,
}

impl RunLoop {
    /// Create a run loop and its posting handle.
    pub fn new() -> (Self, RunLoopHandle) {
        let (sender, receiver) = mpsc::channel();
        (
            Self { receiver },
            RunLoopHandle { sender },
        )
    }

    /// Block the current thread and process tasks until `stop()` is posted.
    pub fn run(self) {
        while let Ok(message) = self.receiver.recv() {
            match message {
                RunLoopMessage::Task(task) => task(),
                RunLoopMessage::Stop => break,
            }
        }
    }
}

impl RunLoopHandle {
    /// Post a task into the run loop.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce() + Send + 'static,
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
