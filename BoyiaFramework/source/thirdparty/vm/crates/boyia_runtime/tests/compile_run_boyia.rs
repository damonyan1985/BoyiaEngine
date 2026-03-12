//! Integration test: compile and run Boyia code via BoyiaRuntime.
//! Verifies init -> compile -> run_exe_file flow.

use boyia_runtime::BoyiaRuntime;

#[test]
fn test_runtime_init_minimal() {
    let rt = BoyiaRuntime::create_minimal_for_test();
    assert!(!rt.vm().is_null());
    std::mem::forget(rt);
}

#[test]
fn test_runtime_init_only() {
    let rt = BoyiaRuntime::create();
    assert!(!rt.vm().is_null());
    std::mem::forget(rt); // skip Drop to see if crash is in destroy_vm
}

#[test]
fn test_compile_run_empty_script() {
    let rt = BoyiaRuntime::create();
    rt.compile("");
    rt.run_exe_file();
    rt.consume_micro_task();
}

#[test]
fn test_compile_run_var_assign() {
    let rt = BoyiaRuntime::create();
    rt.compile("var x = 1;");
    rt.run_exe_file();
    rt.consume_micro_task();
}
