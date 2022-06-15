package com.boyia.app.shell.home

import android.app.Activity
import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.RelativeLayout
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.shell.model.BoyiaAppListModel.LoadCallback
import com.boyia.app.shell.R
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.setting.BoyiaSettingFragment
import com.boyia.app.shell.setting.BoyiaSettingModule
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideCallback
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideListener
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.dp
import com.mikepenz.iconics.IconicsDrawable
import com.mikepenz.iconics.typeface.library.googlematerial.GoogleMaterial

class BoyiaHomeFragment(private val module: HomeModule): BaseFragment() {
    companion object {
        const val TAG = "BoyiaHomeFragment"
        const val GRID_SPAN_NUM = 3
        const val HEADER_BG_COLOR = 0xFFEDEDED.toInt()
        const val CONTAINER_BG_COLOR = 0xFF4F4F4F.toInt()
    }

    private var headerView: BoyiaHomeHeader? = null
    private var footerView: BoyiaHomeFooter? = null
    private var middleView: RecyclerView? = null
    private var appListAdapter: BoyiaAppListAdapter? = null
    private var rootLayout: FrameLayout? = null
    private var contentLayout: LinearLayout? = null
    private var maskView: FrameLayout? = null
    private var listener: SlideListener? = null

    // 要防止onCreateView被多次调用
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout
        }

        contentLayout = LinearLayout(context)
        contentLayout?.setBackgroundColor(CONTAINER_BG_COLOR)
        contentLayout?.orientation = LinearLayout.VERTICAL

        listener = object: SlideListener {
            private var callback: SlideCallback? = null
            override fun onStart(value: Float) {
                BoyiaLog.d(TAG, "maskView start")
                if (maskView != null) {
                    return
                }
                maskView = FrameLayout(requireContext())
                val lp = FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT
                )

                maskView?.setBackgroundColor(0x88000000.toInt())
                maskView?.alpha = value

                maskView?.setOnClickListener {
                    doHide()
                }
                rootLayout?.addView(maskView, lp)
            }

            override fun onSlide(value: Float, opacity: Float) {
                rootLayout!!.x = value
                maskView?.alpha = opacity
                BoyiaLog.d(TAG, "rootLayout.x = ${rootLayout!!.x} value = $value and opacity = $opacity")
            }

            override fun doHide() {
                callback?.doHide()
            }

            override fun onEnd(show: Boolean) {
                if (!show) {
                    rootLayout?.removeView(maskView)
                    maskView = null
                }
            }

            override fun setSlideCallback(cb: SlideCallback) {
                callback = cb
            }
        }

        headerView = BoyiaHomeHeader(context, module, listener!!)
        footerView = BoyiaHomeFooter(context)

        appListAdapter = BoyiaAppListAdapter(requireContext(), module)
        middleView = RecyclerView(requireContext())
        middleView?.layoutManager = StaggeredGridLayoutManager(GRID_SPAN_NUM, RecyclerView.VERTICAL)
        middleView?.adapter = appListAdapter
        middleView?.addItemDecoration(BoyiaGridSpaceItem(12.dp))

        val headerParam: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                130.dp
        )
        contentLayout?.addView(headerView, headerParam)

        val middleParam: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        )
        contentLayout?.addView(middleView, middleParam)
        contentLayout?.addView(footerView)

        //activity?.window?.statusBarColor = HEADER_BG_COLOR
        //HandlerFoundation.setStatusbarTextColor(activity, true)

        module.clear()
        module.loadAppList(object : LoadCallback {
            override fun onLoaded() {
                appListAdapter?.notifyDataSetChanged()
            }
        })

        rootLayout = FrameLayout(requireContext())
        rootLayout?.layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        )
        val lp = LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        )
        lp.topMargin = BoyiaUtils.getStatusBarHeight(context as Activity)
        rootLayout?.addView(contentLayout, lp)
        rootLayout?.setBackgroundColor(HEADER_BG_COLOR)
        //rootLayout?.fitsSystemWindows = true
        //rootLayout?.clipToPadding = false
        return rootLayout
    }

    override fun canPop(): Boolean {
        return false
    }

    class BoyiaHomeHeader(context: Context?, module: HomeModule, listener: SlideListener) : RelativeLayout(context) {
        private var imageSetting: ImageView? = null

        init {
            imageSetting = ImageView(context)
            imageSetting?.setImageResource(R.drawable.gear)
            // 设置图像颜色
            imageSetting?.setColorFilter(Color.BLACK)
            setBackgroundColor(HEADER_BG_COLOR)

            val settingParam = LayoutParams(
                    50.dp,
                    50.dp
            )
            settingParam.addRule(CENTER_VERTICAL)
            settingParam.addRule(ALIGN_PARENT_RIGHT)
            settingParam.rightMargin = 20.dp

            imageSetting?.setOnClickListener {
                val ctx = module.getContext()
                if (ctx != null) {
                    val settingModule = ModuleManager.instance().getModule(ModuleManager.SETTING) as BoyiaSettingModule
                    settingModule.show(ctx)
                    settingModule.setSlideListener(listener)
                }
            }

            val searchLayout = context?.let { FrameLayout(it) }
            searchLayout?.setBackgroundColor(Color.WHITE)
            searchLayout?.setOnClickListener {
                // 启动搜索页
                module.showSearch()
            }

            val searchParam = LayoutParams(
                    540.dp,
                    70.dp
            )
            //searchParam.addRule(ALIGN_PARENT_BOTTOM)
            searchParam.addRule(CENTER_VERTICAL)
            searchParam.addRule(ALIGN_PARENT_LEFT)
            //searchParam.bottomMargin = 20.dp
            searchParam.leftMargin = 60.dp

            val drawable = IconicsDrawable(context!!, GoogleMaterial.Icon.gmd_search)
                    .apply {
                        sizeXPx = 36.dp
                        sizeYPx = 36.dp
                    }

            val searchImageView = ImageView(context)
            searchImageView.setImageDrawable(drawable)
            val searchImageParam = FrameLayout.LayoutParams(
                    36.dp,
                    36.dp
            )
            searchImageParam.leftMargin = 20.dp
            searchImageParam.gravity = Gravity.CENTER_VERTICAL
            searchLayout?.addView(searchImageView, searchImageParam)

            CommonFeatures.setViewRadius(searchLayout!!, 35.dp)

            addView(imageSetting, settingParam)
            addView(searchLayout, searchParam)
        }
    }

    class BoyiaHomeFooter(context: Context?) : RelativeLayout(context) {

    }
}