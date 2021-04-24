use jni::JNIEnv;
use jni::objects::{JClass};
use jni::sys::jstring;

#[no_mangle]
pub extern "system" fn Java_com_boyia_app_core_BoyiaCoreJNI_nativeInitSdk(env: JNIEnv, clzz: JClass) -> jstring {
  let output = env.new_string("hello boyia rust").expect("Couldn't create java string!");
  output.into_inner()
}