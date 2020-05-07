from appium import webdriver

class AutoTestDriver:
  def __init__(self, url, desired_capabilities):
    self.driver = webdriver.Remote(url, desired_capabilities)

  def find_view_by_id(self, id):
    self.driver.find_element_by_id(f'com.boyia.app:id/{id}')