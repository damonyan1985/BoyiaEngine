use crate::service::{IService};
use std::collections::HashMap;

pub trait IServiceWrapper {
  fn register(&self);
}

pub struct ServiceWrapper<T> {
  service: Option<T>,
}

impl<T> ServiceWrapper<T> {
  pub fn new(service: T) -> Self {
    Self {
      service: Some(service),
    }
  }
}

impl<T> IServiceWrapper for ServiceWrapper<T> 
where
  T: IService
{
  fn register(&self) {}
}

pub struct ServiceManager {
  services: HashMap<String, Box<dyn IServiceWrapper>>,
}

impl ServiceManager {
  // 初始化ServiceManager
  pub fn new() -> ServiceManager {
    ServiceManager { services: HashMap::new() }
  }

  // 注册服务
  pub fn register(&mut self, key: String, service: Box<dyn IServiceWrapper>) {
    self.services.insert(key, service);
  }
}