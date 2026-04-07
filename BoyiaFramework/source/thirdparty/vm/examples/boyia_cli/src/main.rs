//! Example: use BoyiaRuntime to compile and run a Boyia program from `script/main.boyia`
//! (under this crate directory, resolved via `CARGO_MANIFEST_DIR`).
//! Prints results via `BY_Log` and exercises async/File/Https builtins.
//!
//! If the program hangs or crashes, see CRASH_ANALYSIS.md. You can set
//! env BOYIA_INIT_MINIMAL=1 to skip builtin classes (faster init, fewer deps)
//! and narrow down whether the crash is in init.

mod builtins;
mod run_loop;
mod runner;
mod task_thread;
mod thread_pool;

use runner::BoyiaRunner;
use std::sync::{Arc, Mutex};
use task_thread::TaskThread;

fn run_task_thread_demo() {
    println!("[TaskThread] Starting task thread...");
    let task_thread = TaskThread::start();
    let handle = task_thread.handle();
    let values = Arc::new(Mutex::new(Vec::new()));

    let values_1 = Arc::clone(&values);
    handle
        .post_task(move |_| {
            println!("[TaskThread] Run task 1");
            values_1.lock().unwrap().push("task-1".to_string());
        })
        .expect("failed to post task 1");

    let values_2 = Arc::clone(&values);
    task_thread
        .post_task(move |_| {
            println!("[TaskThread] Run task 2");
            values_2.lock().unwrap().push("task-2".to_string());
        })
        .expect("failed to post task 2");

    task_thread.join().expect("failed to join task thread");
    println!("[TaskThread] Completed tasks: {:?}", *values.lock().unwrap());
}

fn main() {
    //run_task_thread_demo();
    println!("Boyia CLI: compile and run script\n");

    println!("[1] Creating runtime...");
    let runner = BoyiaRunner::create();
    if !runner.is_ready() {
        eprintln!("Error: VM init returned null");
        return;
    }
    println!("[3] VM ready.");

    let script_path = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("script")
        .join("main.boyia");
    let script = std::fs::read_to_string(&script_path).unwrap_or_else(|e| {
        panic!("failed to read {}: {e}", script_path.display());
    });

    println!("[4] Compiling script...");
    runner
        .compile(&script, Some(&script_path))
        .expect("failed to compile script on task thread");
    println!("[6] Running script...");

    // run_task_thread_demo();
    println!("\nDone.");
    // When main returns, runner is dropped -> Drop stops task thread and joins it -> "BoyiaRunner exit!!!"
}
