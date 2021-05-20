use crate::service::{IService};
use std::collections::HashMap;

pub struct ServiceManager {
  services: HashMap<String, Box<dyn IService>>,
}

impl ServiceManager {
  // 初始化ServiceManager
  pub fn new() -> ServiceManager {
    ServiceManager { services: HashMap::new() }
  }

  // 注册服务
  pub fn register(&mut self, key: String, service: Box<dyn IService>) {
    self.services.insert(key, service);
  }
}