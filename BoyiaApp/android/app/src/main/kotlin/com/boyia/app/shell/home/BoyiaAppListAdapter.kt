package com.boyia.app.shell.home

import android.content.Context

import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.launch.BoyiaAppInfo
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.shell.api.IBoyiaHomeLoader
import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.update.DownloadData
import com.boyia.app.shell.update.DownloadUtil

class BoyiaAppListAdapter(
        private val context: Context,
        private val loader: IBoyiaHomeLoader): RecyclerView.Adapter<BoyiaAppItemHolder>() {
    companion object {
        const val TAG = "BoyiaAppListAdapter"
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): BoyiaAppItemHolder {
        BoyiaLog.d(TAG, "onCreateViewHolder")
        return BoyiaAppItemHolder(BoyiaAppItemView(context))
    }

    override fun onBindViewHolder(holder: BoyiaAppItemHolder, position: Int) {
        val view = holder.itemView as BoyiaAppItemView
        val item = loader.appItem(position)
        view.appNameView?.text = item.name
        view.appIconView?.load(item.cover)
        view.container?.setOnClickListener {
            BoyiaAppLauncher.launcher().launch(getAppInfo(item))
        }

        //view.initDownloadMask(TEST_URL)
        view.initDownloadMask(item.url!!)
        BoyiaLog.d(TAG, "onBindViewHolder-" + loader.appItem(position).toString())
    }

    override fun getItemCount(): Int {
        return loader.appListCount()
    }

    private fun getAppInfo(appItem: BoyiaAppItem): BoyiaAppInfo {
        val name = BoyiaUtils.getStringMD5(appItem.url)

        val info = DownloadData()
        info.fileName = name

        val list = DownloadUtil.getDownloadList(info)
        val module = ModuleManager.instance().getModule(ModuleManager.IPC) as IPCModule

        return BoyiaAppInfo(
                appItem.appId,
                appItem.versionCode,
                appItem.name,
                list[0].filePath,
                appItem.url,
                appItem.cover,
                module.getBinder()
        )
    }
}