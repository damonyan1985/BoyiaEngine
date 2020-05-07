import os
from appium import webdriver
import subprocess
import threading
import time

app_package_name = 'com.boyia.app'
appium_server_host = '127.0.0.1'
appium_server_port = '9999'

desired_capabilities = {
  'platformName': 'Android',
  'platformVersion': '10',
  'deviceName': '4739aed6',
  'appPackage': app_package_name,
  'appActivity': 'com.boyia.app.BoyiaActivity',
  'unicodeKeyBoard': False,
  'resetKeyBoard': False,
  'automationName': 'UiAutomator2',
  'noReset': True
}

start_appium_cmd = (
  f'appium -a {appium_server_host} -p {appium_server_port}'
)

#  启动appium server
class AppiumThread(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)

  def run(self):
    print('AppiumThread run')
    os.system(start_appium_cmd)

# 实际测试容器
class TestThread(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)

  def run(self):
    print('test TestThread')
    time.sleep(10)
    print('connect to server')
    driver = AutoTestDriver(f'http://{appium_server_host}:{appium_server_port}/wd/hub', desired_capabilities)
    #driver = webdriver.Remote(f'http://127.0.0.1:{appium_server_port}/wd/hub', desired_capabilities)


def main():
  #os.system(start_appium_cmd)
  appiumThread = AppiumThread()
  appiumThread.start()

  testThread = TestThread()
  testThread.start()
  print('test')
  testThread.join()
  appiumThread.join()

if __name__ == '__main__':
  main()