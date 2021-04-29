// 引入server模块
//mod server;
#[macro_use]
extern crate log;
extern crate android_logger;


use jni::JNIEnv;
use jni::objects::{JClass};
use jni::sys::jstring;

use log::Level;
use android_logger::Config;

// server
use std::thread;
use std::{convert::Infallible, net::SocketAddr};
use hyper::{Body, Request, Response, Server};
use hyper::service::{make_service_fn, service_fn};
use tokio::runtime::Runtime;

async fn handle(_: Request<Body>) -> Result<Response<Body>, Infallible> {
  info!("http handle respone");
  Ok(Response::new("Just Test Hello, World!!!!\n".into()))
}

async fn start_server() {
  let addr = SocketAddr::from(([0, 0, 0, 0], 3000));

  // 从handle创建一个服务
  let make_svc = make_service_fn(|_conn| async {
      Ok::<_, Infallible>(service_fn(handle))
  });

  let server = Server::bind(&addr).serve(make_svc);

  // 运行server
  if let Err(e) = server.await {
    error!("start server error: {}", e);
  }
}

#[no_mangle]
pub extern "system" fn Java_com_boyia_app_core_BoyiaCoreJNI_nativeInitSdk(env: JNIEnv, _clzz: JClass) -> jstring {
  // 启动服务
  //start_main();
  android_logger::init_once(
    Config::default()
      .with_min_level(Level::Info) // limit log level
      .with_tag("BoyiaSdk") // logs will show under BoyiaSdk tag
  );

  info!("this is a info {}", "message");
  error!("this is printed by default");

  // start server
  thread::spawn(|| {
    info!("start boyia sdk server thread");
    Runtime::new()
      .expect("start tokio runtime error")
      .block_on(start_server())
  });

  let output = env.new_string("hello boyia rust").expect("Couldn't create java string!");
  output.into_inner()
}