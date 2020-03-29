import os
import operator

android_dir = os.getcwd()
maven_dir = os.path.join(android_dir, 'maven')
maven_project_dir = os.path.join(maven_dir, 'BoyiaMaven')
core_dir = os.path.join(android_dir, 'core')
apk_build_dir = os.path.join(android_dir, 'app', 'build')

pull_core_maven_cmd = (
  'cd maven && git clone git@github.com:damonyan1985/BoyiaMaven.git'
)

# add --rerun-tasks to resolute externalNativeBuild not exist
upload_core_library_cmd = (
  'cd core && gradle :core:uploadArchives --rerun-tasks'
)

upload_util_library_cmd = (
  'cd util && gradle :util:uploadArchives'
)

upload_loader_library_cmd = (
  'cd loader && gradle :loader:uploadArchives'
)

upload_leaker_library_cmd = (
  'cd leaker && gradle :leaker:uploadArchives'
)

build_app_cmd = (
  'cd app && gradle assembleDebug'
)

install_apk_cmd = (
  'adb install -t app/build/outputs/apk/boyia/debug/app-boyia-debug.apk'
)

launch_app_cmd = (
  'adb shell am start -n com.boyia.app/com.boyia.app.BoyiaActivity'
)

def del_file(path, isDel):
  ls = os.listdir(path)
  for i in ls:
    c_path = os.path.join(path, i)
    print("i=" + i)
    if operator.eq(i, '.git') == True:
      continue
    if operator.eq(i, '.gitignore') == True:
      continue
    print("c_path=" + c_path);  
    if os.path.isdir(c_path):
      del_file(c_path, True)
    else:
      os.remove(c_path)
  if isDel:      
    os.rmdir(path)    

def main():
  print('maven_dir=' + maven_dir)
  if os.path.exists(maven_dir) == False:
    os.mkdir(maven_dir)
    os.system(pull_core_maven_cmd)
  
  del_file(maven_project_dir, False)
  if os.path.exists(apk_build_dir) == True:
    del_file(apk_build_dir)
  os.system(upload_util_library_cmd)
  os.system(upload_loader_library_cmd)
  #os.system(upload_core_native_sync_cmd)
  os.system(upload_leaker_library_cmd)
  os.system(upload_core_library_cmd)
  os.system(build_app_cmd)
  os.system(install_apk_cmd)
  os.system(launch_app_cmd)
  print('Build Finished!')

if __name__ == '__main__':
  main()