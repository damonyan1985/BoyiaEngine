package com.boyia.app.shell.home

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.RelativeLayout
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.boyia.app.shell.model.BoyiaAppListModel.LoadCallback
import com.boyia.app.common.utils.BoyiaUtils.dp
import com.boyia.app.shell.R
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.ModuleManager

class BoyiaHomeFragment(private val loader: IHomeModule): Fragment() {
    companion object {
        const val GRID_SPAN_NUM = 3
        const val HEADER_BG_COLOR = 0xFFEDEDED.toInt()
        const val CONTAINER_BG_COLOR = 0xFF4F4F4F.toInt()
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
            return rootLayout
        }

        rootLayout = LinearLayout(context)
        rootLayout?.setBackgroundColor(CONTAINER_BG_COLOR)
        rootLayout?.orientation = LinearLayout.VERTICAL

        headerView = BoyiaHomeHeader(context, loader)
        footerView = BoyiaHomeFooter(context)

        appListAdapter = BoyiaAppListAdapter(requireContext(), loader)
        middleView = RecyclerView(requireContext())
        middleView?.layoutManager = StaggeredGridLayoutManager(GRID_SPAN_NUM, RecyclerView.VERTICAL)
        middleView?.adapter = appListAdapter
        middleView?.addItemDecoration(BoyiaGridSpaceItem(dp(12)))

        val headerParam: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                dp(130)
        )
        rootLayout?.addView(headerView, headerParam)

        val middleParam: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        )
        rootLayout?.addView(middleView, middleParam)
        rootLayout?.addView(footerView)

        activity?.window?.statusBarColor = HEADER_BG_COLOR
        HandlerFoundation.setStatusbarTextColor(activity, true)

        loader.clear()
        loader.loadAppList(object : LoadCallback {
            override fun onLoaded() {
                appListAdapter?.notifyDataSetChanged()
            }
        })

        return rootLayout
    }

    class BoyiaHomeHeader(context: Context?, module: IHomeModule) : RelativeLayout(context) {
        private var imageSetting: ImageView? = null

        init {
            imageSetting = ImageView(context)
//            val bitmap = IconicsDrawable(context!!, FontAwesome.Icon.faw_cog)
//                    .apply {
//                        sizeXPx = dp(50)
//                        sizeYPx = dp(50)
//                    }
//                    .toBitmap()
            imageSetting?.setImageResource(R.drawable.gear)
            // 设置图像颜色
            imageSetting?.setColorFilter(Color.BLACK)
            setBackgroundColor(HEADER_BG_COLOR)

            val settingParam = LayoutParams(
                    dp(50),
                    dp(50)
            )
            settingParam.addRule(ALIGN_PARENT_BOTTOM)
            settingParam.addRule(ALIGN_PARENT_RIGHT)
            settingParam.bottomMargin = dp(20)
            settingParam.rightMargin = dp(20)

            imageSetting?.setOnClickListener {
                val ctx = module.getContext()
                if (ctx != null) {
                    val settingModule = ModuleManager.instance().getModule(ModuleManager.SETTING)
                    settingModule?.show(ctx)
                }
            }

            addView(imageSetting, settingParam);
        }
    }

    class BoyiaHomeFooter(context: Context?) : RelativeLayout(context) {

    }
}