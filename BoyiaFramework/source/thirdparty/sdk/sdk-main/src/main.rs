mod web;

#[macro_use]
extern crate log;

#[macro_use]
extern crate lazy_static;

use std::thread;
use tokio::runtime::Runtime;
use sdk_util::logger::logger_init;
use web::server::start_server;
use std::collections::HashMap;

trait Test {
 fn test(&self);
}

struct T1<T>
where T: Test 
{
  m: HashMap<String, Box<T>>,
}

fn main() {
  logger_init();

  info!("this is a info {}", "message");
  error!("this is printed by default");

  // start server
  let handle = thread::spawn(|| {
    info!("start boyia sdk server thread");
    Runtime::new()
      .expect("start tokio runtime error")
      .block_on(start_server())
  });

  handle.join().expect("thread has panicked");
}