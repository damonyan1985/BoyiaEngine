use log::Level;
use android_logger::Config;

pub fn logger_init() {
  android_logger::init_once(
    Config::default()
      .with_min_level(Level::Info) // limit log level
      .with_tag("BoyiaSdk") // logs will show under BoyiaSdk tag
  );
}