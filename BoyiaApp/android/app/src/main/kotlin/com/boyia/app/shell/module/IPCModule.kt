package com.boyia.app.shell.module

import android.os.IBinder

interface IPCModule : IModule {
    fun getBinder(): IBinder?
}