package com.boyia.app.shell.setting

import com.boyia.app.shell.module.IModule
import com.boyia.app.shell.module.IModuleContext
import java.lang.ref.WeakReference

class BoyiaSettingModule : IModule {
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

        context = WeakReference(ctx)
        fragment = BoyiaSettingFragment(this)

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(ctx.rootId(), fragment!!)
        fragmentTransaction.commit()
    }

    override fun dispose() {
        val context = context?.get() ?: return

        val fragmentTransaction = context.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.remove(fragment!!)
        fragmentTransaction.commit();

        fragment = null
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