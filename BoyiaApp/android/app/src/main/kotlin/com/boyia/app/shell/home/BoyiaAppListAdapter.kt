package com.boyia.app.shell.home

import android.content.Context

import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.launch.BoyiaAppInfo
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.shell.api.IBoyiaHomeLoader
import com.boyia.app.shell.model.BoyiaAppItem

class BoyiaAppListAdapter(
        private val context: Context,
        private val loader: IBoyiaHomeLoader): RecyclerView.Adapter<BoyiaAppItemHolder>() {
    companion object {
        const val TAG = "BoyiaAppListAdapter"
        const val TEST_URL = "https://klxxcdn.oss-cn-hangzhou.aliyuncs.com/histudy/hrm/media/bg3.mp4"
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): BoyiaAppItemHolder {
        BoyiaLog.d(TAG, "onCreateViewHolder")
        return BoyiaAppItemHolder(BoyiaAppItemView(context))
    }

    override fun onBindViewHolder(holder: BoyiaAppItemHolder, position: Int) {
        val view = holder.itemView as BoyiaAppItemView
        val item = loader.appItem(position)
        view.appNameView?.text = item.name
        BoyiaImager.loadImage(item.cover, view.appIconView)
        view.container?.setOnClickListener {
            BoyiaAppLauncher.launch(getAppInfo(item));
        }

        view.initDownloadMask(TEST_URL)
        BoyiaLog.d(TAG, "onBindViewHolder-" + loader.appItem(position).toString())
    }

    override fun getItemCount(): Int {
        return loader.appListCount()
    }

    private fun getAppInfo(appItem: BoyiaAppItem): BoyiaAppInfo {
        return BoyiaAppInfo(
                appItem.appId,
                appItem.versionCode,
                appItem.name,
                "",
                appItem.url,
                appItem.cover,
                null
        )
    }
}