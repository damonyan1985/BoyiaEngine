pub trait IService : Sync + Send {
  fn register(&self);
}