
import os
import sys

sys.path.append('..')
import build.android_build as android_build

appium_tool_install_cmds = (
  'npm install -g appium && '
  'npm install -g appium-doctor && '
  'pip3 install Appium-Python-Client'
)

def exec_appium_install():
  if os.system('appium') != 0:
    os.system(appium_tool_install_cmds)

def main():
  print('test')
  android_build.add_boyia_app_android_config()

if __name__ == '__main__':
  main()
