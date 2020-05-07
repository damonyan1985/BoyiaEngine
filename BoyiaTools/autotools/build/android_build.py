import os

current_path = os.getcwd()
project_path = os.path.abspath(os.path.join(os.getcwd(), "../../.."))
android_sdk_path = os.getenv('ANDROID_HOME')
android_ndk_path = os.getenv('NDK_HOME')
boyia_app_android_path = os.path.join(project_path, 'BoyiaApp/android')
boyia_app_android_config_path = os.path.join(boyia_app_android_path, 'local.properties')
boyia_app_maven_path = os.path.join(boyia_app_android_path, 'maven')
boyia_app_library_path = os.path.join(boyia_app_android_path, 'library')

boyia_app_sdk_config = (
  f'sdk.dir={android_sdk_path}\n'
  f'ndk.dir={android_ndk_path}\n'
  f'maven_dir={boyia_app_maven_path}\n'
  f'library_dir={boyia_app_library_path}\n'
)

# 写入sdk配置
def add_boyia_app_android_config():
  print(boyia_app_sdk_config)
  if os.path.exists(boyia_app_android_config_path) == False:
    open(boyia_app_android_config_path, 'w').write(boyia_app_sdk_config)
