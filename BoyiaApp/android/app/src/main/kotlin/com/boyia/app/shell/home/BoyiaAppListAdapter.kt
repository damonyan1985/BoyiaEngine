package com.boyia.app.shell.home

import android.content.Context
import java.util.ArrayList;

import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.shell.model.BoyiaAppItem

class BoyiaAppListAdapter(private val context: Context?): RecyclerView.Adapter<BoyiaAppItemHolder>() {
    companion object {
        const val TAG = "BoyiaAppListAdapter"
    }

    private var appList: ArrayList<BoyiaAppItem> = ArrayList()

    fun appendList(list: ArrayList<BoyiaAppItem>) {
        BoyiaLog.d(TAG, "appendList-" +list?.toString())
        appList.addAll(list)
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): BoyiaAppItemHolder {
        BoyiaLog.d(TAG, "onCreateViewHolder")
        return BoyiaAppItemHolder(BoyiaAppItemView(context))
    }

    override fun onBindViewHolder(holder: BoyiaAppItemHolder, position: Int) {
        val view = holder.itemView as BoyiaAppItemView
        view.appNameView?.setText(appList[position].name)
        BoyiaImager.loadImage(appList[position].cover, view.appIconView)

        BoyiaLog.d(TAG, "onBindViewHolder-" + appList[position].toString())
    }

    override fun getItemCount(): Int {
        BoyiaLog.d(TAG, "getItemCount-" +appList.size)
        return appList.size
    }
}