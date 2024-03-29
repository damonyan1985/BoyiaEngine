# Boyia Rust编译指南
## 背景
1. C/C++足够灵活开放，适合开放核心程序，核心程序都是小而美的代码，只需要细心管理内存和线程安全即可
2. 由于C++没有很好的管理库工程的工具，在处理第三方库是需要花大量精力去进行编译和
适配，安全及性能上也得不到保证
3. 本工程主要是开放本地web服务来做http/https代理适配功能，以及更多底层功能API开发

## 环境搭建

## 常见错误

1. 提示_不稳定
```
naming constants with `_` is unstable
```
2. 无法混合编译c文件, 例如rustls中依赖的ring，找不到指定的C编译器
```
跨端编译需要设置CC环境变量如android
CC_aarch64_linux_android=c:/xxx/clang140.exe
只有clang140能解决找不到libunwind的问题, 对应NDK版本android-ndk-r25c
同时需要将rust-tools中的编译器目录配置到path

如果没有找到aarch64-linux-android-ar.exe
可以从早期版本中复制过去就OK了
```
3. 最好把rust_tools/ndk/arm64/bin配置到环境变量中
4. 编译打包
```
1. IOS模拟器编译，cargo build --target x86_64-apple-ios --release
2. IOS真机编译，cargo build --target aarch64-apple-ios --release
3. Android编译，cargo build --target aarch64-linux-android --release
```
5.Linux与mac下直接跑android_build.py就能自动生成cargo config, windows下可以直接粘贴如下配置
```
[source.crates-io]
registry = "https://github.com/rust-lang/crates.io-index"
replace-with = 'sjtu'

[source.ustc]
registry = "https://mirrors.ustc.edu.cn/crates.io-index/"

[source.sjtu]
registry = "https://mirrors.sjtug.sjtu.edu.cn/git/crates.io-index"

[target.aarch64-linux-android]
ar="{Your project dir}\\BoyiaEngine\\BoyiaTools\\rust-tools\\ndk\\arm64\\bin\\aarch64-linux-android-ar.exe"
linker="{Your project dir}\\BoyiaEngine\\BoyiaTools\\rust-tools\\ndk\\arm64\\bin\\aarch64-linux-android-clang.cmd"
```
