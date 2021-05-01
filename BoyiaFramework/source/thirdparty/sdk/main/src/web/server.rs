use std::thread;

// hyper import
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

pub fn start_server_thread() {
  // start server
  thread::spawn(|| {
    info!("start boyia sdk server thread");
    Runtime::new()
      .expect("start tokio runtime error")
      .block_on(start_server())
  });
}