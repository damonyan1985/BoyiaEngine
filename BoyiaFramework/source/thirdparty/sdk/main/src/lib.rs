// 引入server模块
//mod server;

use jni::JNIEnv;
use jni::objects::{JClass};
use jni::sys::jstring;
//use crate::server::start_main;

#[no_mangle]
pub extern "system" fn Java_com_boyia_app_core_BoyiaCoreJNI_nativeInitSdk(env: JNIEnv, _clzz: JClass) -> jstring {
  // 启动服务
  //start_main();

  let output = env.new_string("hello boyia rust").expect("Couldn't create java string!");
  output.into_inner()
}