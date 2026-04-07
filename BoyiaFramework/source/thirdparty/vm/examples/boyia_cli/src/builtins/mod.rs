//! CLI-specific builtin classes (File, Https).

#[path = "async.rs"]
mod r#async;

pub mod file;
pub mod https;
pub mod zip;
