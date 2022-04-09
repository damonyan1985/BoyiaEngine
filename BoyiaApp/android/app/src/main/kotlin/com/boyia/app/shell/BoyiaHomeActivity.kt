package com.boyia.app.shell

import android.graphics.Color
import android.os.Bundle
import android.os.PersistableBundle
import android.view.View
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentTransaction
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.home.BoyiaHomeFragment

// 主页面，用来呈现应用列表信息
class BoyiaHomeActivity: AppCompatActivity() {
    companion object {
        const val TAG = "BoyiaHomeActivity"
    }
    private var rootView: FrameLayout? = null

//    activity已经创建，并且persistableMode设置persistAcrossReboots才会调用
//    override fun onCreate(savedInstanceState: Bundle?, persistentState: PersistableBundle?) {
//        super.onCreate(savedInstanceState, persistentState)
//    }

    override fun onCreate(bundle: Bundle?) {
        super.onCreate(bundle)
        BoyiaLog.d(TAG, "BoyiaHomeActivity onCreate")
        initHome()
    }

    private fun initHome() {
        BoyiaLog.d(TAG, "BoyiaHomeActivity initHome")
        rootView = FrameLayout(this)
        rootView?.id = View.generateViewId()
        rootView?.setBackgroundColor(Color.BLUE)

        var homeFragment = BoyiaHomeFragment()

        var fragmentTransaction = supportFragmentManager.beginTransaction()
        fragmentTransaction.add(rootView!!.id, homeFragment)
        fragmentTransaction.commit()

        setContentView(rootView)
    }
}