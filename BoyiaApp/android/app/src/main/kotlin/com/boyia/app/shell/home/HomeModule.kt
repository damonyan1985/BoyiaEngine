package com.boyia.app.shell.home

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.model.BoyiaAppListModel
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.IModuleContext
import java.lang.ref.WeakReference

// module类似presenter
class HomeModule: IHomeModule {
    private var context: WeakReference<IModuleContext>? = null
    private var appListModel: BoyiaAppListModel? = null
    private var fragment: BoyiaHomeFragment? = null

    override fun init() {
        appListModel = BoyiaAppListModel()
    }

    override fun show(ctx: IModuleContext) {
        context = WeakReference(ctx)
        fragment = BoyiaHomeFragment(this)

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(ctx.rootId(), fragment!!)
        fragmentTransaction.commit()
    }

    override fun hide() {
        val ctx = context?.get() ?: return
        fragment ?: return

        if (ctx.getActivity().supportFragmentManager.isDestroyed) {
            return
        }

        val fragmentTransaction = ctx.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.remove(fragment!!)
        fragmentTransaction.commit()

        fragment = null
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