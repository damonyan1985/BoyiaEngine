package com.boyia.app.shell.home

import android.content.Context

import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.shell.api.IBoyiaHomeLoader

class BoyiaAppListAdapter(
        private val context: Context?,
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
        view.appNameView?.text = loader.appItem(position).name
        BoyiaImager.loadImage(loader.appItem(position).cover, view.appIconView)

        BoyiaLog.d(TAG, "onBindViewHolder-" + loader.appItem(position).toString())
    }

    override fun getItemCount(): Int {
        return loader.appListCount()
    }
}