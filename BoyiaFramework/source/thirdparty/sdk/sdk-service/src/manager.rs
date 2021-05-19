use crate::service::{IService};
use std::collections::HashMap;

pub struct ServiceManager<T> 
where T: IService
{
  services: HashMap<String, Box<T>>,
}

impl<T> ServiceManager<T>
where T: IService 
{
  // 初始化ServiceManager
  pub fn new() -> ServiceManager<T> {
    ServiceManager { services: HashMap::new() }
  }

  // 注册服务
  pub fn register(&mut self, key: String, service: Box<T>) {
    self.services.insert(key, service);
  }
}