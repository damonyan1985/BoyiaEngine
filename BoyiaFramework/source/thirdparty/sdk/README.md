# Boyia Rust编译指南

## 环境搭建

## 常见错误

1. 提示_不稳定
```
naming constants with `_` is unstable
```
2. 无法混合编译c文件,例如rustls中依赖的ring
```
windows中需要设置环境变量CC=c:/xxx/clang90.exe
同时需要将rust-tools中的编译器目录配置到path
```
