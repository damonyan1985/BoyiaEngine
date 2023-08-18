package com.boyia.app.shell.module

import androidx.appcompat.app.AppCompatActivity
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.util.PermissionCallback

interface IModuleContext {
    fun rootId(): Int
    fun getActivity(): AppCompatActivity
    fun pickImage(loader: IPickImageLoader) {}

    fun sendNotification(callback: PermissionCallback) {}
}