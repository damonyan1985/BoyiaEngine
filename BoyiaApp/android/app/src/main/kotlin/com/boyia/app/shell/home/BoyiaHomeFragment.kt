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

//    override fun onCreate(savedInstanceState: Bundle?) {
//        super.onCreate(savedInstanceState)
//    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        var layout = LinearLayout(context)

        headerView = BoyiaHomeHeader(context)
        footerView = BoyiaHomeFooter(context)

        appListAdapter = BoyiaAppListAdapter(context, loader)
        middleView = RecyclerView(requireContext())
        middleView?.layoutManager = StaggeredGridLayoutManager(GRID_SPAN_NUM, RecyclerView.VERTICAL)
        middleView?.adapter = appListAdapter
        middleView?.addItemDecoration(BoyiaGridSpaceItem(dp(12)))

        layout.addView(headerView)
        layout.addView(middleView)
        layout.addView(footerView)

//        BoyiaAppListModel.requestAppList(object : LoadCallback {
//            override fun onResult(result: String?) {
//                var data: BoyiaAppListData = BoyiaJson.jsonParse(result, BoyiaAppListData::class.java)
//                appListAdapter?.appendList(data.apps)
//            }
//        })
        loader.loadAppList(object : LoadCallback {
            override fun onLoaded() {
                appListAdapter?.notifyDataSetChanged()
            }
        })
        return layout
    }

    class BoyiaHomeHeader(context: Context?) : RelativeLayout(context) {

    }

    class BoyiaHomeFooter(context: Context?) : RelativeLayout(context) {

    }
}