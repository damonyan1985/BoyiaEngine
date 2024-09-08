import os
import operator
import sys
import shutil
import platform

current_path = os.getcwd()

# boyia project path
project_path = os.path.abspath(os.path.join(os.getcwd(), '../../..'))
# 需要在环境变量中添加[ANDROID_HOME]与[NDK_HOME]，与[CARGO_HOME]即.cargo目录所在路径
cargo_path = os.getenv('CARGO_HOME')
android_sdk_path = os.getenv('ANDROID_HOME')
android_ndk_path = os.getenv('NDK_HOME')

boyia_rust_sdk_path = os.path.join(
    project_path, 'BoyiaFramework/source/thirdparty/sdk/sdk-main')

boyia_app_android_path = os.path.join(project_path, 'BoyiaApp/android')

# change the workspace
# os.chdir(boyia_app_android_path)
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

boyia_rust_tools_path = os.path.join(project_path, 'BoyiaTools/rust-tools')
boyia_rust_tools_ndk_path = os.path.join(boyia_rust_tools_path, 'ndk')
boyia_rust_tools_config_path = os.path.join(cargo_path, '.cargo/config')
boyia_rust_sdk_lib_path = os.path.join(core_dir, 'libs/arm64-v8a')
boyia_rust_sdk_lib_name = 'libsdk_main.so'

# write in sdk config
boyia_app_sdk_config = (
    f'sdk.dir={android_sdk_path}\n'
    f'ndk.dir={android_ndk_path}\n'
    f'maven_dir={maven_project_dir}\n'
    f'library_dir={boyia_app_library_path}\n'
)

# rust sdk config not in windows
boyia_rust_sdk_config = (
    f'[source.crates-io]\n'
    f'registry = "https://github.com/rust-lang/crates.io-index"\n'
    f'replace-with = \'ustc\'\n'
    f'\n'
    f'[source.ustc]\n'
    f'registry = "https://mirrors.ustc.edu.cn/crates.io-index/"\n'
    f'\n'
    f'[target.aarch64-linux-android]\n'
    f'ar="{boyia_rust_tools_ndk_path}/arm64/bin/aarch64-linux-android-ar"\n'
    f'linker="{boyia_rust_tools_ndk_path}/arm64/bin/aarch64-linux-android-clang"\n'
)

pull_core_maven_cmd = (
    'cd maven && git clone git@github.com:damonyan1985/BoyiaMaven.git'
)

permission_gradlew_cmd = (
    f'chmod +x {gradle_cmd}'
)

# add --rerun-tasks to resolute externalNativeBuild not exist
# archives = 'uploadArchives', support for gradle 6.0
archives = 'publishReleasePublicationToMavenRepository'
upload_core_library_cmd = (
    f'{gradle_cmd} :core:{archives} --rerun-tasks'
)

upload_util_library_cmd = (
    f'{gradle_cmd} :util:{archives}'
)

upload_loader_library_cmd = (
    f'{gradle_cmd} :loader:{archives}'
)

upload_debugger_library_cmd = (
    f'{gradle_cmd} :debug:{archives}'
)

upload_bytrace_library_cmd = (
    f'{gradle_cmd} :bytrace:publish'
)

# build_app_cmd = (
#     f'{gradle_cmd} -Dhttp.proxyHost=127.0.0.1 -Dhttp.proxyPort=7890 -Dhttps.proxyHost=127.0.0.1 -Dhttps.proxyPort=7890 assembleDebug'
# )

build_app_cmd = (
    f'{gradle_cmd} assembleDebug'
)

# build app bundle
build_bundle_cmd = (
    f'{gradle_cmd} bundle'
)

install_apk_cmd = (
    f'adb install -t {boyia_app_android_path}/app/build/outputs/apk/boyia/debug/app-boyia-debug1.0-DEV.apk'
)

launch_app_cmd = (
    'adb shell am start -n com.boyia.app.shell/com.boyia.app.shell.BoyiaHomeActivity'
)

create_rust_environment = (
    f'python3 {android_ndk_path}/build/tools/make_standalone_toolchain.py --api 28 --arch arm64 --install-dir {boyia_rust_tools_ndk_path}/arm64'
)

boyia_rust_sdk_cmd = (
    'cargo build --target aarch64-linux-android --release'
)

boyia_install_ndk_cmd = (
    'rustup target add aarch64-linux-android'
)

# 为android工程添加配置，即local.properities
def add_boyia_app_android_config():
    print(boyia_app_sdk_config)
    if os.path.exists(boyia_app_android_config_path) == False:
        open(boyia_app_android_config_path, 'w').write(boyia_app_sdk_config)

# 删除目录下所以文件
def del_file(path, isDel):
    ls = os.listdir(path)
    canDel = isDel
    for i in ls:
        # 拼接路径
        c_path = os.path.join(path, i)
        print('i=' + i)
        # 判断目录名是否需要过滤
        if operator.eq(i, 'bytrace') == True:
            canDel = False
            continue
        if operator.eq(i, '.git') == True:
            continue
        if operator.eq(i, '.gitignore') == True:
            continue
        print("c_path=" + c_path)
        if os.path.isdir(c_path):
            canDel = canDel and del_file(c_path, True)
        else:
            os.remove(c_path)
    if isDel and canDel:
        os.rmdir(path)
    return canDel

# 编译boyia工程中的rust子工程
def do_build_boyia_rust_sdk():
    os.chdir(boyia_rust_sdk_path)
    os.system(boyia_rust_sdk_cmd)
    # 创建libs/arm64-v8a目录
    if os.path.exists(boyia_rust_sdk_lib_path) == False:
        os.makedirs(boyia_rust_sdk_lib_path)
    rust_build_lib_path = os.path.join(
        boyia_rust_sdk_path, f'target/aarch64-linux-android/release/{boyia_rust_sdk_lib_name}')
    shutil.copy(rust_build_lib_path, os.path.join(
        boyia_rust_sdk_lib_path, boyia_rust_sdk_lib_name))


def do_single_cmd(cmd):
    print('do_single_cmd cmd=' + cmd)
    if cmd == 'sdk':
        do_build_boyia_rust_sdk()
    elif cmd == 'bundle':
        os.system(build_bundle_cmd)
    elif cmd == 'trace':
        os.chdir(boyia_app_android_path)
        os.system(upload_bytrace_library_cmd)   

# 安装boyia工程中的rust ndk环境
def install_boyia_rust_environment():
    if os.path.exists(boyia_rust_tools_path) == True:
        return   
    os.makedirs(boyia_rust_tools_ndk_path)
    print(create_rust_environment)
    os.system(create_rust_environment)

# 安装boyia工程中rust配置
def install_boyia_rust_config():
    if os.path.exists(boyia_rust_tools_config_path) == True:
        return
    print(boyia_rust_sdk_config)    
    # 非windows系统添加配置
    if platform.system().lower() != 'windows':
        open(boyia_rust_tools_config_path, 'w').write(boyia_rust_sdk_config)
    os.system(boyia_install_ndk_cmd)

# 前提是系统已经安装了rust开发环境
def install_and_build_rust():
    install_boyia_rust_environment()
    print(cargo_path)
    install_boyia_rust_config()
    do_build_boyia_rust_sdk()

# 启动入口, 
# build sdk, python3 android_build.py sdk
# build apk, python3 android_build.py
def main():
    length = len(sys.argv)
    print('arguments length=' + str(length))
    if length >= 2:
        do_single_cmd(sys.argv[1])
        return

    # 编译boyia rust库
    install_and_build_rust()

    # 切换编译目录编译Android工程
    os.chdir(boyia_app_android_path)
    # 判断local.properties是否存在
    if os.path.exists(boyia_app_android_config_path) == False:
        add_boyia_app_android_config()

    print('maven_dir=' + maven_dir)
    if os.path.exists(maven_dir) == False:
        os.mkdir(maven_dir)
        #os.system(pull_core_maven_cmd)

    if os.path.exists(maven_project_dir) == False:
        os.mkdir(maven_project_dir)    

    del_file(maven_project_dir, False)

    # 删除编译目录
    if os.path.exists(apk_build_dir) == True:
        del_file(apk_build_dir, True)

    print('system=' + platform.system().lower())
    # 如果不是windows系统, gradlew需要添加权限
    if platform.system().lower() != 'windows':
        os.system(permission_gradlew_cmd)

    # 编译公共库
    os.system(upload_util_library_cmd)
    # 编译资源加载库
    os.system(upload_loader_library_cmd)
    # 编译调试库
    os.system(upload_debugger_library_cmd)
    # 编译引擎核心库
    os.system(upload_core_library_cmd)
    # 编译日志追踪库
    # os.system(upload_bytrace_library_cmd)
    # 编译壳工程
    os.system(build_app_cmd)
    # 安装壳工程
    os.system(install_apk_cmd)
    # 启动壳工程
    os.system(launch_app_cmd)
    print('Build Finished!')
    print(boyia_app_sdk_config)


if __name__ == '__main__':
    main()
