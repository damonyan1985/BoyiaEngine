package com.boyia.app.shell.login

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.model.BoyiaLoginModel
import com.boyia.app.shell.model.BoyiaUserInfo
import com.boyia.app.shell.module.ILoginModule
import com.boyia.app.shell.module.IModuleContext
import java.lang.ref.WeakReference

class LoginModule: ILoginModule {
    companion object {
        const val TAG = "LoginModule"
    }

    private var context: WeakReference<IModuleContext>? = null
    private var fragment: BoyiaLoginFragment? = null
    private var model: BoyiaLoginModel? = null
    private var listeners = mutableListOf<LoginListener>()

    override fun init() {
        model = BoyiaLoginModel()
    }

    override fun show(ctx: IModuleContext) {
        context = WeakReference(ctx)

        fragment = BoyiaLoginFragment(this)

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        //fragmentTransaction.setCustomAnimations(R.anim.page_in, R.anim.page_out)
        //fragmentTransaction.add(ctx.rootId(), fragment!!)
        fragmentTransaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
        fragmentTransaction.add(ctx.rootId(), fragment!!)
        fragmentTransaction.addToBackStack(null)
        fragmentTransaction.commit()
    }

    fun login(name: String?, password: String?) {
        model?.login(name, password) { info ->
            BoyiaLog.d(TAG, "login nickname = ${info.nickname}")
            listeners.forEach {
                it.onLogined(info)
            }

            MainScheduler.mainScheduler().sendJob {
                hide()
            }
        }
    }

    override fun hide() {
        fragment?.hide()
        listeners.clear()
    }

    override fun dispose() {
        hide()
    }

    open fun addLoginLisnter(listener: LoginListener) {
        listeners.add(listener)
    }

    interface LoginListener {
        fun onLogined(info: BoyiaUserInfo);
    }
}