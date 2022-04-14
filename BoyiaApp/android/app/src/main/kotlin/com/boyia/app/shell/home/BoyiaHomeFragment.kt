package com.boyia.app.shell.home

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.RelativeLayout
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.boyia.app.shell.api.IBoyiaHomeLoader
import com.boyia.app.shell.model.BoyiaAppListModel.LoadCallback
import com.boyia.app.common.utils.BoyiaUtils.dp

class BoyiaHomeFragment(private val loader: IBoyiaHomeLoader): Fragment() {
    companion object {
        const val GRID_SPAN_NUM = 3
    }

    private var headerView: BoyiaHomeHeader? = null
    private var footerView: BoyiaHomeFooter? = null
    private var middleView: RecyclerView? = null
    private var appListAdapter: BoyiaAppListAdapter? = null
    private var rootLayout: LinearLayout? = null

    // 要防止onCreateView被多次调用
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout;
        }

        rootLayout = LinearLayout(context)

        headerView = BoyiaHomeHeader(context)
        footerView = BoyiaHomeFooter(context)

        appListAdapter = BoyiaAppListAdapter(requireContext(), loader)
        middleView = RecyclerView(requireContext())
        middleView?.layoutManager = StaggeredGridLayoutManager(GRID_SPAN_NUM, RecyclerView.VERTICAL)
        middleView?.adapter = appListAdapter
        middleView?.addItemDecoration(BoyiaGridSpaceItem(dp(12)))

        rootLayout?.addView(headerView)
        rootLayout?.addView(middleView)
        rootLayout?.addView(footerView)

        loader.loadAppList(object : LoadCallback {
            override fun onLoaded() {
                appListAdapter?.notifyDataSetChanged()
            }
        })
        return rootLayout
    }

    class BoyiaHomeHeader(context: Context?) : RelativeLayout(context) {

    }

    class BoyiaHomeFooter(context: Context?) : RelativeLayout(context) {

    }
}