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

// 主页面，用来呈现应用列表信息
class BoyiaHomeActivity: BoyiaShellActivity() {
    companion object {
        const val TAG = "BoyiaHomeActivity"
    }

//    activity已经创建，并且persistableMode设置persistAcrossReboots才会调用
//    override fun onCreate(savedInstanceState: Bundle?, persistentState: PersistableBundle?) {
//        super.onCreate(savedInstanceState, persistentState)
//    }

//    private val pickLauncher = registerForActivityResult(ActivityResultContracts.GetContent()){ uri: Uri? ->
//        uri?.let { it ->
//            BoyiaLog.d("tag", it.toString())
//        }
//    }

    private val pickerLoaders = mutableListOf<IPickImageLoader>()
    private val pickLauncher = CommonFeatures.registerPickerImage(this) { path ->
        BoyiaLog.d(CommonFeatures.TAG, "registerPickerImage path = $path")
        pickerLoaders.forEach { it
            it.onImage(path)
        }

        pickerLoaders.clear()
    }

    private val permissionCallbacks = mutableListOf<PermissionCallback>()

    override fun onCreate(bundle: Bundle?) {
        super.onCreate(bundle)
        BoyiaLog.d(TAG, "BoyiaHomeActivity onCreate")
        initHome()
        initNotifyService()
        BoyiaUtils.setStatusbarTransparent(this)
        BoyiaLog.d(TAG, "BoyiaHomeActivity is main process:" + ProcessUtil.isMainProcess())
    }

    private fun initHome() {
        BoyiaLog.d(TAG, "BoyiaHomeActivity initHome")
        val ipcModule = ModuleManager.instance().getModule(ModuleManager.IPC)
        ipcModule?.show(this)

        val homeModule = ModuleManager.instance().getModule(ModuleManager.HOME)
        homeModule?.show(this)

        HandlerFoundation.setStatusbarTextColor(this, true)
    }

    override fun pickImage(loader: IPickImageLoader) {
        pickerLoaders.add(loader)
        if (BoyiaPermissions.requestPhotoPermissions(this)) {
            pickLauncher.launch(null)
        }
    }

    override fun sendNotification(callback: PermissionCallback) {
        permissionCallbacks.add(callback)
        if (NotificationManagerCompat.from(this).areNotificationsEnabled()
                || BoyiaPermissions.requestNotificationPermissions(this)) {
            permissionCallbacks.forEach {
                it()
            }

            permissionCallbacks.clear()
        }
    }

    private fun initNotifyService() {
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

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when(requestCode) {
            BoyiaPermissions.CAMERA_REQUEST_CODE -> {
                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    pickLauncher.launch(null)
                }
            }
            BoyiaPermissions.NOTIFICATION_REQUEST_CODE -> {
                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    permissionCallbacks.forEach {
                        it()
                    }

                    permissionCallbacks.clear()
                }
            }
        }
    }
}