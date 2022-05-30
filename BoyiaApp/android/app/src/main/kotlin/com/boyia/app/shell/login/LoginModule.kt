package com.boyia.app.shell.login

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.module.ILoginModule
import com.boyia.app.shell.module.IModuleContext
import java.lang.ref.WeakReference

class LoginModule: ILoginModule {
    private var context: WeakReference<IModuleContext>? = null
    private var fragment: BoyiaLoginFragment? = null

    override fun init() {
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

    override fun hide() {
        fragment?.hide()
    }

    override fun dispose() {
        fragment?.hide()
    }
}