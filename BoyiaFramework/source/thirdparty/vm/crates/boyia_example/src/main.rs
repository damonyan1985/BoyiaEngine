//! Example: use BoyiaRuntime to compile and run a simple Boyia program.
//! Prints results of simple calculations via BY_Log.
//!
//! If the program hangs or crashes, see CRASH_ANALYSIS.md. You can set
//! env BOYIA_INIT_MINIMAL=1 to skip builtin classes (faster init, fewer deps)
//! and narrow down whether the crash is in init.

use boyia_runtime::BoyiaRuntime;

fn main() {
    println!("Boyia Example: compile and run script\n");

    println!("[1] Creating runtime...");
    let mut rt = BoyiaRuntime::new();
    println!("[2] Initializing VM...");
    rt.init();
    if rt.vm().is_null() {
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
BY_Log(123);
"#;
//     let script = r#"class Printer { fun say(msg) { BY_Log(msg); } }
// "#;

    println!("[4] Compiling script...");
    rt.compile(script);
    //println!("[5] Caching VM code...");
    //rt.cache_code();
    println!("[6] Running script...");
    //rt.run_exe_file();
    // println!("[6] Consuming micro tasks...");
    // rt.consume_micro_task();

    println!("\nDone.");
}
