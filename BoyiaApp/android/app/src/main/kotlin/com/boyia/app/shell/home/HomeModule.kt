package com.boyia.app.shell.home

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.api.IBoyiaHomeLoader
import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.model.BoyiaAppListModel
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.IModuleContext
import java.lang.ref.WeakReference

// module类似presenter
class HomeModule: IHomeModule {
    private var context: WeakReference<IModuleContext>? = null
    private var appListModel: BoyiaAppListModel? = null

    override fun init() {
        appListModel = BoyiaAppListModel()
    }

    override fun show(ctx: IModuleContext) {
        context = WeakReference(ctx)
        val homeFragment = BoyiaHomeFragment(this)

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(ctx.rootId(), homeFragment)
        fragmentTransaction.commit()
    }

    override fun dispose() {
        appListModel?.clear()
    }

    override fun getContext(): IModuleContext? {
        return context?.get()
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