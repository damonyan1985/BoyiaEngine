//! Simple thread pool built on top of `TaskThread<()>`.

#![allow(dead_code)]

use crate::run_loop::RunLoopError;
use crate::task_thread::TaskThread;
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};

struct Worker {
    task_thread: TaskThread<()>,
    load: Arc<AtomicUsize>,
}

impl Worker {
    fn new() -> Self {
        Self {
            task_thread: TaskThread::start(),
            load: Arc::new(AtomicUsize::new(0)),
        }
    }

    fn load(&self) -> usize {
        self.load.load(Ordering::Acquire)
    }

    fn post_task<F>(&self, task: F) -> Result<(), RunLoopError>
    where
        F: FnOnce() + Send + 'static,
    {
        self.load.fetch_add(1, Ordering::AcqRel);
        let load = Arc::clone(&self.load);
        let result = self.task_thread.post_task(move |_| {
            struct TaskGuard {
                load: Arc<AtomicUsize>,
            }

            impl Drop for TaskGuard {
                fn drop(&mut self) {
                    self.load.fetch_sub(1, Ordering::AcqRel);
                }
            }

            let _guard = TaskGuard { load };
            task();
        });

        if result.is_err() {
            self.load.fetch_sub(1, Ordering::AcqRel);
        }

        result
    }

    fn stop(&self) -> Result<(), RunLoopError> {
        self.task_thread.stop()
    }

    fn join(self) -> std::thread::Result<()> {
        self.task_thread.join()
    }
}

/// Thread pool that lazily creates `TaskThread<()>` workers up to a configured maximum.
pub struct ThreadPool {
    max_thread_count: usize,
    workers: Mutex<Vec<Worker>>,
    stopped: AtomicBool,
}

impl ThreadPool {
    /// Create a pool with a maximum worker count. Workers are created on demand.
    pub fn new(max_thread_count: usize) -> Self {
        assert!(
            max_thread_count > 0,
            "max_thread_count must be greater than zero"
        );
        Self {
            max_thread_count,
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
            workers.iter().enumerate().find(|(_, worker)| worker.load() == 0)
        {
            index
        } else if workers.len() < self.max_thread_count {
            workers.push(Worker::new());
            workers.len() - 1
        } else {
            // 在“所有线程都忙、并且已经达到最大线程数，不能再新建线程”时，
            // 从现有 worker 里找出负载最小的那个线程，然后把任务投递给它。
            workers
                .iter()
                .enumerate()
                .min_by_key(|(_, worker)| worker.load())
                .map(|(index, _)| index)
                .expect("thread pool must have at least one worker")
        };

        workers[worker_index].post_task(task)
    }

    /// Request all worker threads to stop after previously queued tasks finish.
    pub fn stop(&self) -> Result<(), RunLoopError> {
        self.stopped.store(true, Ordering::Release);
        let workers = self
            .workers
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        for worker in workers.iter() {
            worker.stop()?;
        }
        Ok(())
    }

    /// Stop all workers and wait for them to exit.
    pub fn join(self) -> std::thread::Result<()> {
        let workers = self
            .workers
            .into_inner()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        for worker in workers {
            worker.join()?;
        }
        Ok(())
    }
}
