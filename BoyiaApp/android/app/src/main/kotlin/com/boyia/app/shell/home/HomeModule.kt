package com.boyia.app.shell.home

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.api.IBoyiaHomeLoader
import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.model.BoyiaAppListModel
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.IModuleContext

// module类似presenter
class HomeModule: IHomeModule, IBoyiaHomeLoader {
    private var appListModel: BoyiaAppListModel? = null

    override fun init() {
        appListModel = BoyiaAppListModel()
    }

    override fun show(context: IModuleContext) {
        val homeFragment = BoyiaHomeFragment(this)

        val fragmentTransaction = context.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(context.rootId(), homeFragment)
        fragmentTransaction.commit()
    }

    override fun dispose() {
        appListModel?.clear()
    }

    override fun loadAppList(callback: BoyiaAppListModel.LoadCallback) {
        appListModel?.requestAppList(callback)
    }

    override fun appListCount(): Int {
        BoyiaLog.d(BoyiaAppListAdapter.TAG, "getItemCount-" + appListModel?.appList!!.size)
        return appListModel?.appList!!.size
    }

    override fun appItem(index: Int): BoyiaAppItem {
        return appListModel?.appList!![index]
    }

    override fun clear() {
        appListModel?.appList?.clear()
    }
}