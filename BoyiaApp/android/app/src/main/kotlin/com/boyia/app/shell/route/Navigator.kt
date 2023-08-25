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
            context.getActivity().supportFragmentManager.beginTransaction().apply {
                setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                add(context.rootId(), it, tag)
                addToBackStack(null)
                commit()
            }
        }

    }
}