use std::thread;

// hyper import
use std::{convert::Infallible, net::SocketAddr};
use hyper::{Body, Method, Request, Response, Server, StatusCode};
use hyper::service::{make_service_fn, service_fn};
use tokio::runtime::Runtime;
use sdk_service::manager::{ServiceManager, ServiceWrapper};
use sdk_service::service::{IService};
use std::sync::RwLock;

pub struct BoyiaWebServer {
  pub serviceManager: RwLock<ServiceManager>,
}

impl BoyiaWebServer {
  pub fn new() -> Self {
    Self {
      serviceManager: RwLock::new(ServiceManager::new()),
    }
  }

  pub fn service<S>(&mut self, service: S) -> &mut Self 
  where
    S: IService + 'static
  {
    self.serviceManager.write().unwrap().register(service.name(), Box::new(ServiceWrapper::new(service)));
    self
  }

  pub fn start(&self) {
    Runtime::new()
      .expect("start tokio runtime error")
      .block_on(self.start_server())
  }

  async fn start_server(&self) {
    let addr = SocketAddr::from(([0, 0, 0, 0], 3000));
  
    // 从handle创建一个服务
    let make_svc = make_service_fn(move |_conn| async {
        Ok::<_, hyper::Error>(service_fn(move |req: Request<Body>| async move {
          Ok::<_, hyper::Error>(
            handle(req)
          )
        }))
    });
  
    let server = Server::bind(&addr).serve(make_svc);
  
    // 运行server
    if let Err(e) = server.await {
      error!("start server error: {}", e);
    }
  }
}

// lazy_static! {
//   pub static ref SERVICE_MANAGER: RwLock<ServiceManager> = RwLock::new(ServiceManager::new());
// }

fn handle(req: Request<Body>) -> Response<Body> {
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

  response
  //Ok(Response::new("Just Test Hello, World!!!!\n".into()))
}

// pub async fn start_server() {
//   let addr = SocketAddr::from(([0, 0, 0, 0], 3000));

//   // 从handle创建一个服务
//   let make_svc = make_service_fn(|_conn| async {
//       Ok::<_, hyper::Error>(service_fn(handle))
//   });

//   let server = Server::bind(&addr).serve(make_svc);

//   // 运行server
//   if let Err(e) = server.await {
//     error!("start server error: {}", e);
//   }
// }

pub fn start_server_thread() {
  // start server
  thread::spawn(|| {
    info!("start boyia sdk server thread");
    // Runtime::new()
    //   .expect("start tokio runtime error")
    //   .block_on(start_server())
    BoyiaWebServer::new()
      .start()
  });
}