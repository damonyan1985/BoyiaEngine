//! Example: use BoyiaRuntime to compile and run a simple Boyia program.
//! Prints results of simple calculations via BY_Log.
//!
//! If the program hangs or crashes, see CRASH_ANALYSIS.md. You can set
//! env BOYIA_INIT_MINIMAL=1 to skip builtin classes (faster init, fewer deps)
//! and narrow down whether the crash is in init.

mod https;
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

    // Boyia class with a method that prints a string: Printer.say(msg) calls BY_Log(msg).
    // Create instance with new(Printer); call p.say("hello") to print (method dispatch runs during compile/execute).
    let script = r#"class Printer { fun say(msg) { BY_Log(msg); } };
class MyAdd { fun add(a, b) { return a+b; } }    
var p = new(Printer);
fun printlog(p, str) {
    p.say(str);
}
printlog(p, "hello");    
printlog(p, "world");

var a = new(MyAdd);
printlog(p, "result:" + a.add(100,202));

var arr1 = ["789", "100"];
printlog(p, arr1.get(0));

var body = {
    "Content-Type" : "application/x-www-form-urlencoded",
    "User-Token" : "1234567890",
    body : "name=test&pwd=test",
    items : ["123", "456"]
};
printlog(p, body.get("body"));

class Util {
    fun newMicrotask(worker) {
        BY_Log("call newMicrotask");
        var task = new(MicroTask);
        BY_Log("call MicroTask create");
        task.init(worker);
        BY_Log("call MicroTask init");
        return task;
    }
}

class PrinterExt extends Printer {
    prop fun multiply(a, b) {
        return a*b;
    }

    prop fun load() {
        Https.load("https://httpbin.org/get", fun(body) {
            var result = body;
            BY_Log("Https.load result: " + body);
        });
    }

    prop async loadAsync() {
        for (var i = 0; i < 10; i=i+1) {
            if (i == 6) {
                break;
            }
            BY_Log("loadAsync loop: " + i);
        }
    
        var result = (await this.loadPromise());
        var test = "hello : " + result;
        BY_Log("loadAsync result: " + test);
        return result;
    }

    prop async loadPromise() {
        BY_Log("run loadPromise");
        Util.newMicrotask(fun(resolve) {
            BY_Log("run newMicrotask");
            Https.load("https://httpbin.org/get", resolve);
        });
    }

    prop fun testLocal() {
        var test = "hello";
        for (var i = 0; i < 10; i=i+1) {
            var test = "world";
            if (i == 6) {
                break;
            }
            BY_Log("testLocal loop: " + i + test);
        }

        BY_Log("testLocal result: " + test);
    }
}

var pe = new(PrinterExt);
printlog(p, pe.multiply(30, 41));
printlog(pe, pe.multiply(30, 42));
BY_Log(123);
//pe.load();

pe.loadAsync();

pe.testLocal();
"#;
//     let script = r#"class Printer { fun say(msg) { BY_Log(msg); } }
// "#;

    println!("[4] Compiling script...");
    runner
        .compile(script)
        .expect("failed to compile script on task thread");
    println!("[6] Running script...");

    // run_task_thread_demo();
    println!("\nDone.");
    // When main returns, runner is dropped -> Drop stops task thread and joins it -> "BoyiaRunner exit!!!"
}
