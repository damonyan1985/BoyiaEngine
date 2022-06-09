package com.boyia.app.shell.search

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IUIModule

class SearchModule : IUIModule {
    override fun init() {
    }

    override fun show(context: IModuleContext) {
        val search = SearchFragment()
        val fragmentTransaction = context.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
        fragmentTransaction.add(context.rootId(), search)
        fragmentTransaction.addToBackStack(null)
        fragmentTransaction.commit()
    }

    override fun hide() {
    }

    override fun dispose() {
    }
}