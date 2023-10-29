package com.boyia.app.shell.login

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.model.BoyiaLoginModel
import com.boyia.app.shell.model.BoyiaUserInfo
import com.boyia.app.shell.module.ILoginModule
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.LoginListener
import com.boyia.app.shell.route.Navigator
import java.lang.ref.WeakReference

class LoginModule: ILoginModule {
    companion object {
        const val TAG = "LoginModule"
    }

    private var context: WeakReference<IModuleContext>? = null
    private var fragment: BoyiaLoginFragment? = null
    private var listeners = mutableListOf<LoginListener>()

    override fun init() {
    }

    override fun show(ctx: IModuleContext) {
        context = WeakReference(ctx)
        fragment = BoyiaLoginFragment(this)
        Navigator(ctx).push(fragment, TAG)
    }

    override fun login(name: String?, password: String?) {
        BoyiaLoginModel.login(name, password) { info ->
            BoyiaLog.d(TAG, "login nickname = ${info.nickname}")
            listeners.forEach {
                it.onLogined(info)
            }

            hide()
        }
    }

    override fun hide() {
        fragment?.hide()
        listeners.clear()
    }

    override fun dispose() {
        hide()
    }

    override fun addLoginListener(listener: LoginListener) {
        listeners.add(listener)
    }


}