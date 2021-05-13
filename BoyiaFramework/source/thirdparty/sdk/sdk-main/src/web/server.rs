use std::thread;

// hyper import
use std::{convert::Infallible, net::SocketAddr};
use hyper::{Body, Method, Request, Response, Server, StatusCode};
use hyper::service::{make_service_fn, service_fn};
use tokio::runtime::Runtime;

async fn handle(req: Request<Body>) -> Result<Response<Body>, hyper::Error> {
  info!("http handle respone");
  let mut response = Response::new(Body::empty());
  match (req.method(), req.uri().path()) {
    (&Method::GET, "/") => {
      *response.body_mut() = Body::from("Try get data to /");
    },
    (&Method::GET, "/main") => {
      *response.body_mut() = Body::from("Try get data to /main");
    },
    (&Method::GET, "/login") => {
      *response.body_mut() = Body::from("Try get data to /login");
    },
    (&Method::GET, "/detail") => {
      *response.body_mut() = Body::from("Try get data to /detail");
    },
    _ => {
      *response.status_mut() = StatusCode::NOT_FOUND;
    },
  }

  Ok(response)
  //Ok(Response::new("Just Test Hello, World!!!!\n".into()))
}

pub async fn start_server() {
  let addr = SocketAddr::from(([0, 0, 0, 0], 3000));

  // 从handle创建一个服务
  let make_svc = make_service_fn(|_conn| async {
      Ok::<_, hyper::Error>(service_fn(handle))
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