package com.boyia.app.shell

import android.graphics.Color
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.coordinatorlayout.widget.CoordinatorLayout
import kotlinx.coroutines.MainScope

/**
 * shell程序共用activity基类
 */
open class BoyiaShellActivity: AppCompatActivity() {
    companion object {
        const val TAG = "BoyiaShellActivity"
    }
    private var rootView: CoordinatorLayout? = null

    override fun onCreate(bundle: Bundle?) {
        super.onCreate(bundle)
        rootView = CoordinatorLayout(this)
        rootView?.id = View.generateViewId()
        rootView?.setBackgroundColor(Color.WHITE)
        rootView?.fitsSystemWindows = false
        rootView?.clipToPadding = false

        setContentView(rootView)
    }

    fun rootId(): Int {
        return rootView?.id!!
    }
}