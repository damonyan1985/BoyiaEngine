//! Simple thread pool built on top of `TaskThread<()>`.

#![allow(dead_code)]

use crate::run_loop::RunLoopError;
use crate::task_thread::TaskThread;
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::Mutex;

/// Thread pool that lazily creates `TaskThread<()>` workers up to a configured maximum.
pub struct ThreadPool {
    max_thread_count: usize,
    /// Prefix for [`TaskThread::start_with_name`]; each worker gets `{prefix}-{unique_id}`.
    thread_name_prefix: String,
    next_worker_id: AtomicUsize,
    workers: Mutex<Vec<TaskThread<()>>>,
    stopped: AtomicBool,
}

impl ThreadPool {
    /// Create a pool with a maximum worker count and default thread name prefix `boyia-pool-worker`.
    pub fn new(max_thread_count: usize) -> Self {
        Self::with_thread_name_prefix(max_thread_count, "boyia-pool-worker")
    }

    /// Create a pool; new workers are named `{prefix}-{n}` with `n` from an internal counter.
    pub fn with_thread_name_prefix(max_thread_count: usize, thread_name_prefix: impl Into<String>) -> Self {
        assert!(
            max_thread_count > 0,
            "max_thread_count must be greater than zero"
        );
        Self {
            max_thread_count,
            thread_name_prefix: thread_name_prefix.into(),
            next_worker_id: AtomicUsize::new(0),
            workers: Mutex::new(Vec::with_capacity(max_thread_count)),
            stopped: AtomicBool::new(false),
        }
    }

    /// Number of worker threads currently started by the pool.
    pub fn thread_count(&self) -> usize {
        self.workers
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner())
            .len()
    }

    /// Maximum number of worker threads the pool may create.
    pub fn max_thread_count(&self) -> usize {
        self.max_thread_count
    }

    /// Post a task. Create a new worker only when all current workers are busy.
    pub fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce() + Send + 'static,
    {
        if self.stopped.load(Ordering::Acquire) {
            return Err(RunLoopError::Stopped);
        }

        let mut workers = self
            .workers
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner());

        if self.stopped.load(Ordering::Acquire) {
            return Err(RunLoopError::Stopped);
        }

        let worker_index = if let Some((index, _)) =
            workers.iter().enumerate().find(|(_, w)| w.load() == 0)
        {
            index
        } else if workers.len() < self.max_thread_count {
            let id = self.next_worker_id.fetch_add(1, Ordering::Relaxed);
            let name = format!("{}-{}", self.thread_name_prefix, id);
            workers.push(TaskThread::start_with_name(name));
            workers.len() - 1
        } else {
            // 在“所有线程都忙、并且已经达到最大线程数，不能再新建线程”时，
            // 从现有 worker 里找出负载最小的那个线程，然后把任务投递给它。
            workers
                .iter()
                .enumerate()
                .min_by_key(|(_, w)| w.load())
                .map(|(index, _)| index)
                .expect("thread pool must have at least one worker")
        };

        workers[worker_index].post_task(move |_| task())
    }

    /// Request all worker threads to stop after previously queued tasks finish.
    pub fn stop(&self) -> Result<(), RunLoopError> {
        self.stopped.store(true, Ordering::Release);
        let workers = self
            .workers
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        for w in workers.iter() {
            w.stop()?;
        }
        Ok(())
    }

    /// Stop all workers and wait for them to exit.
    pub fn join(self) -> std::thread::Result<()> {
        let workers = self
            .workers
            .into_inner()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        for w in workers {
            w.join()?;
        }
        Ok(())
    }
}
