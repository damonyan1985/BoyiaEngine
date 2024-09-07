package com.boyia.app.shell

import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.common.utils.ProcessUtil
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.permission.BoyiaPermissions
import com.boyia.app.shell.service.BoyiaNotifyService
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.PermissionCallback
import java.lang.ref.WeakReference

// 主页面，用来呈现应用列表信息
class BoyiaHomeActivity: BoyiaShellActivity() {
    companion object {
        const val TAG = "BoyiaHomeActivity"
    }

    private val proxy = BoyiaHomeProxy(WeakReference(this))

    override fun onCreate(bundle: Bundle?) {
        super.onCreate(bundle)
        BoyiaLog.d(TAG, "BoyiaHomeActivity onCreate")
        //initHome()
        proxy.onCreate()
        initNotifyService()
        BoyiaUtils.setStatusbarTransparent(this)
        BoyiaLog.d(TAG, "BoyiaHomeActivity is main process:" + ProcessUtil.isMainProcess())
    }

    private fun initNotifyService() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
//            proxy.requestPermissions(BoyiaPermissions.NOTIFICATION_SERVICES_PERMISSIONS) {
//                val intent = Intent(this, BoyiaNotifyService::class.java)
//                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
//                    startForegroundService(intent)
//                } else {
//                    startService(intent)
//                }
//            }
//        }
        val intent = Intent(this, BoyiaNotifyService::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent)
        } else {
            startService(intent)
        }
    }

    override fun onBackPressed() {
        if (supportFragmentManager.backStackEntryCount == 0) {
            super.onBackPressed()
        } else {
            val list = supportFragmentManager.fragments
            val fragment = list.last() as BaseFragment
            if (fragment.canPop()) {
                fragment.hide()
            }
        }
    }

    override fun onDestroy() {
        ModuleManager.instance().hide()
        super.onDestroy()
    }

    // 处理权限
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        proxy.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }
}