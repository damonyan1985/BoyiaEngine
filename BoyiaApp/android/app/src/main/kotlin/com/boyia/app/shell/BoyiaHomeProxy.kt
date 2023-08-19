package com.boyia.app.shell

import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.NotificationManagerCompat
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.permission.BoyiaPermissions
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.PermissionCallback
import java.lang.ref.WeakReference

/**
 * 分离BoyiaHomeActivity功能
 */
class BoyiaHomeProxy(private val context: WeakReference<BoyiaHomeActivity>) : IModuleContext {
    companion object {
        const val TAG = "BoyiaHomeProxy"
    }

    // 提供图库打开功能
    private val pickerLoaders = mutableListOf<IPickImageLoader>()
    private val pickLauncher = CommonFeatures.registerPickerImage(getActivity()) { path ->
        BoyiaLog.d(TAG, "registerPickerImage path = $path")
        pickerLoaders.forEach { it
            it.onImage(path)
        }

        pickerLoaders.clear()
    }

    // 消息通知
    private val notifyCallbacks = mutableListOf<PermissionCallback>()

    fun onCreate() {
        initHome()
    }

    // 初始化IPC功能，以及打开首月
    private fun initHome() {
        BoyiaLog.d(TAG, "BoyiaHomeActivity initHome")
        context.get()?.let {
            val ipcModule = ModuleManager.instance().getModule(ModuleManager.IPC)
            ipcModule?.show(this)

            val homeModule = ModuleManager.instance().getModule(ModuleManager.HOME)
            homeModule?.show(this)

            HandlerFoundation.setStatusbarTextColor(it, true)
        }
    }

    // 打开图库
    override fun pickImage(loader: IPickImageLoader) {
        pickerLoaders.add(loader)
        if (BoyiaPermissions.requestPhotoPermissions(getActivity())) {
            pickLauncher.launch(null)
        }
    }

    // 发送通知，通知程序在回调中处理
    override fun sendNotification(callback: PermissionCallback) {
        notifyCallbacks.add(callback)
        if (NotificationManagerCompat.from(getActivity()).areNotificationsEnabled()
                || BoyiaPermissions.requestNotificationPermissions(getActivity())) {
            BoyiaLog.d(TAG, "sendNotification by permission")
            notifyCallbacks.forEach {
                it()
            }

            notifyCallbacks.clear()
        }
    }

    override fun rootId(): Int {
        return context.get()!!.rootId()
    }

    override fun getActivity(): AppCompatActivity {
        return context.get()!!
    }

    // 处理权限回调
    fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        when(requestCode) {
            BoyiaPermissions.CAMERA_REQUEST_CODE -> {
                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    pickLauncher.launch(null)
                }
            }
            BoyiaPermissions.NOTIFICATION_REQUEST_CODE -> {
                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    BoyiaLog.d(TAG, "sendNotification by granted")
                    notifyCallbacks.forEach {
                        it()
                    }
                    notifyCallbacks.clear()
                }
            }
        }
    }
}