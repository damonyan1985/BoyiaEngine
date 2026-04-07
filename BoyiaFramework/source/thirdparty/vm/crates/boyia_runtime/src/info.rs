//! Compile pipeline state: loaded scripts, current file path/id, string and file compile.
//! Port of `BoyiaCompileInfo` in `BoyiaTools/boyia-ide/boyia/src/lib/BoyiaRuntime.cpp`.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::id_creator::IdCreator;
use boyia_vm::{compile_code, LUintPtr, LVoid};
use std::collections::HashSet;
use std::ffi::CString;
use std::path::Path;

/// Mirrors C++ `BoyiaCompileInfo` (`m_programSet`, `m_currentScriptPath`, `m_currentScriptId`, `compile` / `compileFile`).
pub(crate) struct BoyiaCompileInfo {
    /// C++ `HashMap<HashString, LBool> m_programSet` — scripts already merged.
    program_set: HashSet<String>,
    /// C++ `m_currentScriptId` — from `idCreator()->genIdentify(path)` while compiling a file.
    current_script_id: LUintPtr,
    /// C++ `String m_currentScriptPath` (save/restore around nested `compileFile`).
    current_script_path: String,
    /// Rust CLI: when outer `compile_file` restores an empty path, `BY_Require` still resolves relative to this entry file.
    entry_script_path: String,
}

impl BoyiaCompileInfo {
    pub fn new() -> Self {
        Self {
            program_set: HashSet::new(),
            current_script_id: 0,
            current_script_path: String::new(),
            entry_script_path: String::new(),
        }
    }

    #[allow(dead_code)]
    pub(crate) fn current_script_path(&self) -> &str {
        &self.current_script_path
    }

    #[allow(dead_code)]
    pub(crate) fn current_script_id(&self) -> LUintPtr {
        self.current_script_id
    }

    /// Persist main script path for relative requires (see `BoyiaRuntime::set_entry_script_path`).
    pub fn set_entry_script_path(&mut self, path: &str) {
        let p = Path::new(path);
        self.entry_script_path = std::fs::canonicalize(p)
            .map(|p| p.to_string_lossy().into_owned())
            .unwrap_or_else(|_| path.to_string());
    }

    /// C++ `BoyiaCompileInfo::compile` → `CompileCode(script, vm)`.
    pub fn compile_string(&self, script: &str, vm: *mut LVoid) {
        if vm.is_null() {
            return;
        }
        let script_c = CString::new(script).unwrap_or_default();
        unsafe {
            compile_code(script_c.as_ptr() as *mut _, vm);
        }
    }

    /// Path context for `BY_Require`: active `compile_file` target, else `entry_script_path`, else empty (caller may use CWD).
    pub fn require_path_base(&self) -> &str {
        if !self.current_script_path.is_empty() {
            return &self.current_script_path;
        }
        if !self.entry_script_path.is_empty() {
            return &self.entry_script_path;
        }
        ""
    }

    /// C++ `BoyiaCompileInfo::compileFile`: skip if path seen, read file, `compile`, restore previous path/id.
    pub fn compile_file(&mut self, path: &str, vm: *mut LVoid, id_creator: &mut IdCreator) {
        if vm.is_null() {
            return;
        }

        let dedup_key = std::fs::canonicalize(Path::new(path))
            .map(|p| p.to_string_lossy().into_owned())
            .unwrap_or_else(|_| path.to_string());

        if self.program_set.contains(&dedup_key) {
            return;
        }

        let saved_path = std::mem::take(&mut self.current_script_path);
        let saved_id = self.current_script_id;

        self.current_script_path = dedup_key.clone();
        self.current_script_id = id_creator.gen_ident_by_str(&dedup_key);

        let source = match std::fs::read_to_string(path) {
            Ok(s) => s,
            Err(e) => {
                eprintln!("compile_file: read {}: {}", path, e);
                self.current_script_path = saved_path;
                self.current_script_id = saved_id;
                return;
            }
        };

        if !source.is_empty() {
            self.compile_string(&source, vm);
            self.program_set.insert(dedup_key);
        }

        self.current_script_path = saved_path;
        self.current_script_id = saved_id;
    }
}

impl Default for BoyiaCompileInfo {
    fn default() -> Self {
        Self::new()
    }
}
