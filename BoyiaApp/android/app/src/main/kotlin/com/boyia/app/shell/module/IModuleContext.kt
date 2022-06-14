package com.boyia.app.shell.module

import androidx.appcompat.app.AppCompatActivity

interface IModuleContext {
    fun rootId(): Int
    fun getActivity(): AppCompatActivity
    fun pickImage()
}