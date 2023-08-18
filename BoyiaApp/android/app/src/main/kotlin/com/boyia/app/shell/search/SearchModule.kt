package com.boyia.app.shell.search

import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.model.BoyiaAppSearchModel
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IUIModule
import com.boyia.app.shell.route.Navigator

class SearchModule : IUIModule {
    companion object {
        const val TAG = "SearchModule"
    }
    override fun init() {
    }

    override fun show(context: IModuleContext) {
        Navigator(context).push(SearchFragment(this), TAG)
    }

    override fun hide() {
    }

    override fun dispose() {
    }
}