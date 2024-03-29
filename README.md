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
| CSS Module | Parse css file |
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

### Boyia Blog
[Blog](https://damonyan1985.github.io/)

### Author: Boyia，Contacts: 2512854007@qq.com, If you have any questions, please contact me.

## Donate
### 打赏辛苦费
![image](https://github.com/damonyan1985/Boymue/blob/dev/tools/donate/weixin.jpg)
![image](https://github.com/damonyan1985/Boymue/blob/dev/tools/donate/alipay.jpg)
