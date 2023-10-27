package com.boyia.app.shell.route

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.module.IModuleContext

/**
 * 导航
 */
class Navigator(private val context: IModuleContext) {
    fun push(fragment: BaseFragment?, tag: String) {
        fragment?.let {
            context.getActivity()?.supportFragmentManager?.beginTransaction()?.let {
                it.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                it.add(context.rootId(), fragment, tag)
                it.addToBackStack(null)
                it.commit()
            }
        }

    }
}