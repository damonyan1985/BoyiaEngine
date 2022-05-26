package com.boyia.app.shell.home

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.RelativeLayout
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.model.BoyiaAppListModel.LoadCallback
import com.boyia.app.shell.R
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.IHomeModule
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.setting.BoyiaSettingModule
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideCallback
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideListener
import com.boyia.app.shell.util.dp

class BoyiaHomeFragment(private val loader: IHomeModule): Fragment() {
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

        headerView = BoyiaHomeHeader(context, loader, listener!!)
        footerView = BoyiaHomeFooter(context)

        appListAdapter = BoyiaAppListAdapter(requireContext(), loader)
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

        activity?.window?.statusBarColor = HEADER_BG_COLOR
        HandlerFoundation.setStatusbarTextColor(activity, true)

        loader.clear()
        loader.loadAppList(object : LoadCallback {
            override fun onLoaded() {
                appListAdapter?.notifyDataSetChanged()
            }
        })

        rootLayout = FrameLayout(requireContext())
        val lp = LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        )
        rootLayout?.addView(contentLayout, lp)
        return rootLayout
    }

    class BoyiaHomeHeader(context: Context?, module: IHomeModule, listener: SlideListener) : RelativeLayout(context) {
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
                    50.dp,
                    50.dp
            )
            settingParam.addRule(ALIGN_PARENT_BOTTOM)
            settingParam.addRule(ALIGN_PARENT_RIGHT)
            settingParam.bottomMargin = 20.dp
            settingParam.rightMargin = 20.dp

            imageSetting?.setOnClickListener {
                val ctx = module.getContext()
                if (ctx != null) {
                    val settingModule = ModuleManager.instance().getModule(ModuleManager.SETTING) as BoyiaSettingModule
                    settingModule.show(ctx)
                    settingModule.setSlideListener(listener)
                }
            }

            addView(imageSetting, settingParam);
        }
    }

    class BoyiaHomeFooter(context: Context?) : RelativeLayout(context) {

    }
}