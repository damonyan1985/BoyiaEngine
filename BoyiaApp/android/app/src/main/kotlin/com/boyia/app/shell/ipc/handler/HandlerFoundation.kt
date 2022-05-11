package com.boyia.app.shell.ipc.handler

import android.app.Activity
import android.view.View

object HandlerFoundation {
    fun setStatusbarTextColor(activity: Activity?, light: Boolean) {
        val decor = activity?.window?.decorView
        var ui: Int = decor!!.systemUiVisibility

        ui = if (light) {
            ui.or(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR)
        } else {
            ui.and(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR.inv())
        }

        decor.systemUiVisibility = ui
    }
}