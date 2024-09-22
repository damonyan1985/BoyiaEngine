package com.boyia.app.shell.home

import android.os.Bundle
import androidx.fragment.app.FragmentManager
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.model.BoyiaAppListModel
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.ModuleManager
import java.lang.ref.WeakReference

// module类似presenter
class HomeModule: IHomeModule {
    companion object {
        const val TAG = "HomeModule"
    }

    private var context: WeakReference<IModuleContext>? = null
    private var appListModel: BoyiaAppListModel? = null
    private var fragment: BoyiaHomeFragment? = null

    override fun init() {
        appListModel = BoyiaAppListModel()
    }

    override fun show(ctx: IModuleContext) {
        val frag = ctx.getActivity()?.supportFragmentManager?.findFragmentByTag(TAG)
        if (frag != null) {
            // 全部清空，重新開始
            ctx.getActivity()?.supportFragmentManager?.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
        }

        context = WeakReference(ctx)

        fragment = BoyiaHomeFragment()
        fragment?.setUIModule(this)

        ctx.getActivity()?.supportFragmentManager?.beginTransaction()?.let {
            it.add(ctx.rootId(), fragment!!, TAG)
            it.commit()
        }
    }

    override fun hide() {
        val ctx = context?.get() ?: return
        fragment ?: return

        if (ctx.getActivity()?.supportFragmentManager?.isDestroyed == true) {
            fragment = null
            return
        }

        ctx.getActivity()?.supportFragmentManager?.beginTransaction()?.let {
            it.remove(fragment!!)
            it.commitAllowingStateLoss()
        }

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

    fun showSearch() {
        val ctx = context?.get() ?: return
        val module = ModuleManager.instance().getModule(ModuleManager.SEARCH)
        module?.show(ctx)
    }
}