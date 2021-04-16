import os
import operator

current_path = os.getcwd()
# boyia project path
project_path = os.path.abspath(os.path.join(os.getcwd(), "../../.."))
android_sdk_path = os.getenv('ANDROID_HOME')
android_ndk_path = os.getenv('NDK_HOME')
boyia_app_android_path = os.path.join(project_path, 'BoyiaApp/android')

# change the workspace
os.chdir(boyia_app_android_path)
# android project local.properties path
boyia_app_android_config_path = os.path.join(
    boyia_app_android_path, 'local.properties')
boyia_app_maven_path = os.path.join(boyia_app_android_path, 'maven')
boyia_app_library_path = os.path.join(boyia_app_android_path, 'library')

maven_dir = os.path.join(boyia_app_android_path, 'maven')
maven_project_dir = os.path.join(maven_dir, 'BoyiaMaven')
core_dir = os.path.join(boyia_app_android_path, 'core')
apk_build_dir = os.path.join(boyia_app_android_path, 'app', 'build')
gradle_cmd = os.path.join(boyia_app_android_path, 'gradlew')

# 写入sdk配置
boyia_app_sdk_config = (
    f'sdk.dir={android_sdk_path}\n'
    f'ndk.dir={android_ndk_path}\n'
    f'maven_dir={boyia_app_maven_path}\n'
    f'library_dir={boyia_app_library_path}\n'
)

pull_core_maven_cmd = (
    'cd maven && git clone git@github.com:damonyan1985/BoyiaMaven.git'
)

# add --rerun-tasks to resolute externalNativeBuild not exist
upload_core_library_cmd = (
    f'{gradle_cmd} :core:uploadArchives --rerun-tasks'
)

upload_util_library_cmd = (
    f'{gradle_cmd} :util:uploadArchives'
)

upload_loader_library_cmd = (
    f'{gradle_cmd} :loader:uploadArchives'
)

upload_debugger_library_cmd = (
    f'{gradle_cmd} :debug:uploadArchives'
)

build_app_cmd = (
    f'{gradle_cmd} assembleDebug'
)

install_apk_cmd = (
    f'adb install -t {boyia_app_android_path}/app/build/outputs/apk/boyia/debug/app-boyia-debug.apk'
)

launch_app_cmd = (
    'adb shell am start -n com.boyia.app/com.boyia.app.BoyiaMainActivity'
)


def add_boyia_app_android_config():
    print(boyia_app_sdk_config)
    if os.path.exists(boyia_app_android_config_path) == False:
        open(boyia_app_android_config_path, 'w').write(boyia_app_sdk_config)


def del_file(path, isDel):
    ls = os.listdir(path)
    for i in ls:
        c_path = os.path.join(path, i)
        print("i=" + i)
        if operator.eq(i, '.git') == True:
            continue
        if operator.eq(i, '.gitignore') == True:
            continue
        print("c_path=" + c_path)
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
        del_file(apk_build_dir, True)
    os.system(upload_util_library_cmd)
    os.system(upload_loader_library_cmd)
    # os.system(upload_core_native_sync_cmd)
    os.system(upload_debugger_library_cmd)
    os.system(upload_core_library_cmd)
    os.system(build_app_cmd)
    os.system(install_apk_cmd)
    os.system(launch_app_cmd)
    print('Build Finished!')
    print(boyia_app_sdk_config)


if __name__ == '__main__':
    main()
