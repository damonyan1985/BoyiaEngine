package com.boyia.app.shell.route

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.module.IModuleContext

/**
 * 导航
 */
class Navigator(private val context: IModuleContext) {
    fun push(fragment: BaseFragment?) {
        fragment?.let {
            context.getActivity()?.supportFragmentManager?.beginTransaction()?.let {
                it.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                it.add(context.rootId(), fragment, fragment.customTag())
                it.addToBackStack(null)
                it.commit()
            }
        }
    }

    fun replace(fragment: BaseFragment?) {
        fragment?.let {
            context.getActivity()?.supportFragmentManager?.beginTransaction()?.let {
                it.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                it.replace(context.rootId(), fragment, fragment.customTag())
                it.commit()
            }
        }
    }

    fun pop() {
        val fragmentManager = context.getActivity()?.supportFragmentManager
        fragmentManager?.popBackStack()
    }
}