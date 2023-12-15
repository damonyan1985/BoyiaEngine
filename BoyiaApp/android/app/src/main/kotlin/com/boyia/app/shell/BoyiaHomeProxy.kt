package com.boyia.app.shell

import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.NotificationManagerCompat
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.device.permission.IDevicePermission
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.ipc.handler.HandlerFoundation
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.permission.BoyiaPermissions
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.PermissionCallback
import java.lang.ref.WeakReference
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong

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

    // 权限申请
    private val nextPermissionCode = AtomicInteger(1)
    private val permissionsMap = HashMap<Int, PermissionCallback>()

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

    /**
     * 封装权限控制
     */
    override fun requestPermissions(permissions: Array<String>, onPermissionCallback: PermissionCallback) {
        getActivity()?.let {
            val requestCode = nextPermissionCode.getAndIncrement()
            permissionsMap[requestCode] = onPermissionCallback;
            if (BoyiaPermissions.requestPermission(it, permissions, requestCode)) {
                onPermissionCallback()
            }
        }
    }

    // 打开图库
    override fun pickImage(loader: IPickImageLoader) {
        pickerLoaders.add(loader)
        requestPermissions(IDevicePermission.STORAGE_PERMISSIONS) {
            pickLauncher?.launch(null)
        }
    }

    // 发送通知，通知程序在回调中处理
    override fun sendNotification(callback: PermissionCallback) {
        getActivity()?.let {
            if (NotificationManagerCompat.from(it).areNotificationsEnabled()) {
                callback()
                return
            }

            requestPermissions(BoyiaPermissions.NOTIFICATION_PERMISSIONS, callback)
        }
    }

    override fun rootId(): Int {
        return context.get()!!.rootId()
    }

    override fun getActivity(): AppCompatActivity? {
        return context.get()
    }

    // 处理权限回调
    fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
//        when(requestCode) {
//            BoyiaPermissions.CAMERA_REQUEST_CODE -> {
//                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
//                    pickLauncher?.launch(null)
//                }
//            }
//            BoyiaPermissions.NOTIFICATION_REQUEST_CODE -> {
//                if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
//                    BoyiaLog.d(TAG, "sendNotification by granted")
//                    notifyCallbacks.forEach {
//                        it()
//                    }
//                    notifyCallbacks.clear()
//                }
//            }
//        }
        val permissionCallback = permissionsMap[requestCode]
        permissionCallback?.let {
            if ((grantResults.isNotEmpty()).and(grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                permissionCallback()
            }
        }
        permissionsMap.remove(requestCode)
    }
}