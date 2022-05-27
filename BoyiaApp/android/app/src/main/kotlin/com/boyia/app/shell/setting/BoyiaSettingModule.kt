package com.boyia.app.shell.setting

import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IUIModule
import com.boyia.app.shell.module.ModuleManager
import java.lang.ref.WeakReference

class BoyiaSettingModule : IUIModule {
    private var fragment: BoyiaSettingFragment? = null
    private var context: WeakReference<IModuleContext>? = null

    override fun init() {

    }

    fun setSlideListener(listener: SlideListener) {
        fragment?.setSlideListener(listener)
    }

    override fun show(ctx: IModuleContext) {
        if (fragment != null) {
            return
        }

        if (context == null) {
            context = WeakReference(ctx)
        }
        fragment = BoyiaSettingFragment(this)

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(ctx.rootId(), fragment!!)
        fragmentTransaction.commit()
    }

    override fun hide() {
        val ctx = context?.get() ?: return
        fragment ?: return

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.remove(fragment!!)
        fragmentTransaction.commit()

        fragment = null
    }

    override fun dispose() {

    }

    fun showLogin() {
        val loginModule = ModuleManager.instance().getModule(ModuleManager.LOGIN)
        val ctx = context?.get() ?: return
        loginModule?.show(ctx)
    }

    interface SlideCallback {
        fun doHide()
    }

    interface SlideListener {
        fun onStart(value: Float)
        fun onSlide(value: Float, opacity: Float)
        fun doHide()
        fun onEnd(show: Boolean)

        fun setSlideCallback(cb: SlideCallback)
    }
}