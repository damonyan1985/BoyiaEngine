// use actix_web::{get, post, web, App, HttpResponse, HttpServer, Responder};

// // 首页
// #[get("/")]
// async fn index() -> impl Responder {
//   HttpResponse::Ok().body("Hello world!")
// }

// // 登录页
// #[post("/login")]
// async fn login(req_body: String) -> impl Responder {
//   HttpResponse::Ok().body(req_body)
// }

// async fn manual_hello() -> impl Responder {
//   HttpResponse::Ok().body("Hey there!")
// }

// // 启动服务
// pub async fn start_main() -> std::io::Result<()> {
//   HttpServer::new(|| {
//     App::new()
//       .service(index)
//       .service(login)
//       .route("/hey", web::get().to(manual_hello))
//   })
//   .bind("127.0.0.1:9913")?
//   .run()
//   .await
// }