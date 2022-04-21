package com.boyia.app.shell.home

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.util.AttributeSet
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.TextView
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils.dp
import com.boyia.app.core.BoyiaBridge
import com.boyia.app.core.launch.BoyiaAppInfo
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.loader.image.BoyiaImageView
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.update.Downloader

class BoyiaAppItemView(context: Context, attrs: AttributeSet?) : FrameLayout(context, attrs) {
    companion object {
        const val TAG = "BoyiaAppItemView"
    }

    var container: LinearLayout? = null
    var appIconView: BoyiaImageView? = null
    var appNameView: TextView? = null
    var maskView: BoyiaDownloadMask? = null

    init {
        initItemView()
    }

    constructor(context: Context) : this(context, null)

    private fun initItemView() {
        container = LinearLayout(context)
        container?.orientation = LinearLayout.VERTICAL
        appIconView = BoyiaImageView(context)
        val param: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                dp(224), dp(224)
        )

        container?.addView(appIconView, param)

        appNameView = TextView(context)
        container?.addView(appNameView)
        appNameView?.setTextColor(Color.BLACK)
        appNameView?.setBackgroundColor(Color.WHITE)

        container?.setBackgroundColor(Color.WHITE)

        addView(container)
    }

    /**
     * 初始化下载蒙层, 异步执行, 需要判断是否需要添加蒙层, 此过程需要查库耗时操作
     */
    fun initDownloadMask(url: String) {
        JobScheduler.jobScheduler().sendJob {
            if (!Downloader(null, BoyiaBridge.getAppRoot()).initDownloadData(url)) {
                return@sendJob
            }

            MainScheduler.mainScheduler().sendJob {
                addMaskView(url)
            }
        }
    }

    private fun addMaskView(url: String) {
        // maskview不能使用local value，kotlin语法不允许local被使用在闭包中
        // 即便是加了final修饰也不行，和java不一样
        maskView = BoyiaDownloadMask(url, context, object: BoyiaDownloadMask.DownloadCallback {
            override fun onCompleted() {
                MainScheduler.mainScheduler().sendJob {
                    BoyiaLog.d(TAG, "BoyiaDownloadMask onCompleted")
                    if (maskView != null) {
                        removeView(maskView)
                        maskView = null
                    }
                }
            }
        })

        addView(maskView)
    }
}