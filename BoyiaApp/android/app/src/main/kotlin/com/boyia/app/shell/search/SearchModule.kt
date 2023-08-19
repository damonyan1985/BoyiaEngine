package com.boyia.app.shell.search

import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IUIModule
import com.boyia.app.shell.route.Navigator

/**
 * 搜索模块
 */
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