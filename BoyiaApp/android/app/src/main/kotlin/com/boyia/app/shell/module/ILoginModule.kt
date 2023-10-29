package com.boyia.app.shell.module

import com.boyia.app.shell.login.LoginModule
import com.boyia.app.shell.model.BoyiaUserInfo

interface LoginListener {
    fun onLogined(info: BoyiaUserInfo);
}

interface ILoginModule: IUIModule {
    fun login(name: String?, password: String?)
    fun addLoginListener(listener: LoginListener)
}