//! Integration test: compile and run Boyia code via BoyiaRuntime.
//! Verifies init -> compile -> run_exe_file flow.

use boyia_runtime::BoyiaRuntime;

#[test]
fn test_runtime_new_only() {
    let _rt = BoyiaRuntime::new();
    // no init; drop will run (vm is null so destroy_vm is no-op)
}

#[test]
fn test_runtime_init_minimal() {
    let mut rt = BoyiaRuntime::new();
    rt.init_minimal_for_test();
    assert!(!rt.vm().is_null());
    std::mem::forget(rt);
}

#[test]
fn test_runtime_init_only() {
    let mut rt = BoyiaRuntime::new();
    rt.init();
    assert!(!rt.vm().is_null());
    std::mem::forget(rt); // skip Drop to see if crash is in destroy_vm
}

#[test]
fn test_compile_run_empty_script() {
    let mut rt = BoyiaRuntime::new();
    rt.init();
    rt.compile("");
    rt.run_exe_file();
    rt.consume_micro_task();
}

#[test]
fn test_compile_run_var_assign() {
    let mut rt = BoyiaRuntime::new();
    rt.init();
    rt.compile("var x = 1;");
    rt.run_exe_file();
    rt.consume_micro_task();
}
