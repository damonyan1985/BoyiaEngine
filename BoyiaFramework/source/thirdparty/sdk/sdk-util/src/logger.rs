use log::Level;

#[cfg(target_os = "android")]
use android_logger::Config;

#[cfg(target_os = "android")]
pub fn logger_init() {
  android_logger::init_once(
    Config::default()
      .with_min_level(Level::Info) // limit log level
      .with_tag("BoyiaSdk") // logs will show under BoyiaSdk tag
  );
}

#[cfg(target_os = "windows")]
pub fn logger_init() {
}