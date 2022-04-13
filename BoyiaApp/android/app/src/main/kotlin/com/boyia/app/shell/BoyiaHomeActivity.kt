package com.boyia.app.shell

import android.content.Intent
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.os.Looper
import android.os.PersistableBundle
import android.util.ArrayMap
import android.view.View
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentTransaction
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.home.BoyiaHomeFragment
import com.boyia.app.shell.home.HomeModule
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.service.BoyiaNotifyService
import java.util.concurrent.CopyOnWriteArrayList

// 主页面，用来呈现应用列表信息
class BoyiaHomeActivity: AppCompatActivity(), IModuleContext {
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
        initNotifyService()
    }

    private fun initHome() {
        BoyiaLog.d(TAG, "BoyiaHomeActivity initHome")
        rootView = FrameLayout(this)
        rootView?.id = View.generateViewId()
        rootView?.setBackgroundColor(Color.BLUE)
        setContentView(rootView)

        val homeModule = ModuleManager.instance().getModule(ModuleManager.HOME)
        homeModule?.show(this)
    }

    override fun rootId(): Int {
        return rootView?.id!!
    }

    override fun getActivity(): AppCompatActivity {
        return this
    }

    private fun initNotifyService() {
        val intent = Intent(this, BoyiaNotifyService::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent)
        } else {
            startService(intent)
        }
    }
}