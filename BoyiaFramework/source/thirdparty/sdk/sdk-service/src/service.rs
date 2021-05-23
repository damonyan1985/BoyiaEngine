// 跨线程
pub trait IService : Sync + Send {
  fn name(&self) -> String;
  fn register(&self);
}