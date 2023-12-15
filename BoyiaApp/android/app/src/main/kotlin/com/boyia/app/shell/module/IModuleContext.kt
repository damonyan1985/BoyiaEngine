package com.boyia.app.shell.module

import androidx.appcompat.app.AppCompatActivity
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.util.PermissionCallback

interface IModuleContext {
    // 根视图id
    fun rootId(): Int
    // 当前activity
    fun getActivity(): AppCompatActivity?
    // 权限请求
    fun requestPermissions(permissions: Array<String>, onPermissionCallback: PermissionCallback) {}
    // 打开系统相册
    fun pickImage(loader: IPickImageLoader) {}

    // 发送通知
    fun sendNotification(callback: PermissionCallback) {}
}