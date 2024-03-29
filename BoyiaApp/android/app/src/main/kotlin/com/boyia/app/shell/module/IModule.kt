package com.boyia.app.shell.module

interface IModule {
    // 模块需要初始化相关信息的地方可以写在这里
    fun init()
    // 显示主UI
    fun show(context: IModuleContext)
    // 隐藏
    fun hide()
    // 模块析构
    fun dispose()
}