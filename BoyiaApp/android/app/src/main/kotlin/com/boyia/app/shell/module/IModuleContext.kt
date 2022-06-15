package com.boyia.app.shell.module

import androidx.appcompat.app.AppCompatActivity
import com.boyia.app.shell.api.IPickImageLoader

interface IModuleContext {
    fun rootId(): Int
    fun getActivity(): AppCompatActivity
    fun pickImage(loader: IPickImageLoader)
}