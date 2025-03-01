# ![Logo](https://github.com/damonyan1985/BoyiaEngine/blob/dev/BoyiaApp/android/app/src/main/res/drawable/ic_launcher.png?raw=true) BoyiaEngine

BoyiaEngine is a cross-platform framework for building mobile app, for android, ios and windows.

**Boyia Framework Architecture**:
![Architecture](https://github.com/damonyan1985/BoyiaEngine/blob/dev/BoyiaStudy/images/Architecture.png?raw=true)

## Git user attention

1. Clone the repo from GitHub.
$ git clone git@github.com:damonyan1985/BoyiaEngine.git

## Boyia Framework Modules

|Module|Description|
| ----|------|
| Parser Module | Use tinyxml2 to parse xml layout file|
| Loader Module | Curl will be only engine to implement http bridge for boyia app  |
| DOM Module | DOM module to parse ths layout xml and construct a dom tree with widget components |
| CSS Module | Parse css file with css engine |
| Widget Module | Components of elments which description in XML Layout file |
| Layout Module | Layout widget like HTML Style |
| Painter Module | Render widget with opengl commands |
| VM Module | Parse and Build Boyia language which bind with Widget Tree |

## Framework Threads
1. Platform Thread
```
Android, windows or ios UI Thread
```
2. UI Thread
```
Boyia Runtime and UI Elements run on this thread.
```

3. Render Thread
```
Opengl, Metal and GDI+ run on this thread.
```

4. IO Threads
```
Thread pool for all io operation
```

## API implementation
```
Boyia Engine has three method to implement api with C or C++
1, Use builtin to add a class of global object and its methods, 'BuiltinMapClass' you can refer to .
2, Implement a C function to add to BoyiaRuntime, the example is in BoyiaLib.
3, Add a native class which extends from BoyiaBase, that can be collected by BoyiaGC, but you also needs
a C function in BoyiaLib to do that.
```
```
Boyia引擎有三种方法实现其API，使用C或者C++来实现
1，内建类或对象，可以参考BuiltinMapClass的实现
2，使用C函数包装和实现，具体可以参考BoyiaLib中的接口例子，这些接口最终需要添加到BoyiaRuntime中
3，可以使用C++类来实现功能，但需要从BoyiaBase派生，已保证能被BoyiaGC检测到便能够正常回收，使用这个方法需要配合条款2中的C函数来实现
```

## Boyia Compiler Project(Boyia编译器工程)
```
Open BoyiaTools you can get a compiler project which in BoyiaTools/boyia-ide/boyia.
This project is only support windows platform now, the other platform implements will be late.
```

### Boyia Blog
[Blog](https://damonyan1985.github.io/)

### Author: Boyia，Contacts: 2512854007@qq.com, If you have any questions, please contact me.

## Donate
### 打赏辛苦费
![image](https://github.com/damonyan1985/Boymue/blob/dev/tools/donate/weixin.jpg)
![image](https://github.com/damonyan1985/Boymue/blob/dev/tools/donate/alipay.jpg)
