mod web;

#[macro_use]
extern crate log;

#[macro_use]
extern crate lazy_static;

use sdk_util::logger::logger_init;
use web::server::start_server_thread;

// JNI import
use jni::JNIEnv;
use jni::objects::{JClass};
use jni::sys::jstring;

#[cfg(target_os = "android")]
#[no_mangle]
pub extern "system" fn Java_com_boyia_app_core_BoyiaCoreJNI_nativeInitSdk(env: JNIEnv, _clzz: JClass) -> jstring {
  logger_init();

  info!("this is a info {}", "message");
  error!("this is printed by default");

  // start server
  start_server_thread();

  let output = env.new_string("hello boyia rust").expect("Couldn't create java string!");
  output.into_inner()
}