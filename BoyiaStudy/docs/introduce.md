# 介绍

## Windows环境配置
```
设置环境变量BOYIA_UI_ROOT为工程目录
```

## IOS环境配置

1. 如何在ios framework中创建bundle
```
点击framework工程
右侧栏显示PROJECT和TARGETS
点击右侧栏下方+号按钮进行添加
```
```
target中添加bundle
macOS---->bundle，添加完成后修改bundle的Base SDK 为 iOS
编译时同时编译一下bundle
点击设备旁边的工程选项---->edit scheme---->build---->在目标工程中添加bundle
将bundle加入到工程资源中
build phases---->Copy Bundle Resource---->将编译好的bundle添加进来
```
2. 头文件引入
```
如果需要添加头文件引用目录，使用Build Settings中的Header Search Path选项进行添加
```

## Android环境变量配置
1. 配置ANDROID_HOME
2. 配置NDK_HOME

## Rust环境搭建
