package com.boyia.app.shell.service

import android.app.*
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.os.Process
import androidx.core.app.NotificationCompat
import com.boyia.app.shell.BoyiaConstants
import com.boyia.app.shell.R

class BoyiaNotifyService: Service() {
    companion object {
        const val BOYIA_APP_CHANNEL_ID = "boyia_app_channel_id"
        const val BOYIA_APP_CHANNEL_NAME = "boyia_app_channel_name"
    }

    override fun onCreate() {
        super.onCreate()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        //return super.onStartCommand(intent, flags, startId)
        initNotificationManager()
        startForeground(Process.myPid(), createNotification())
        // 服务被中止后不再重启该服务
        return START_NOT_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    private fun initNotificationManager() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val nm = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            // BOYIA_APP_CHANNEL_NAME不能少，不然崩溃
            nm.createNotificationChannel(NotificationChannel(
                    BOYIA_APP_CHANNEL_ID,
                    BOYIA_APP_CHANNEL_NAME,
                    NotificationManager.IMPORTANCE_HIGH))
        }
    }

    private fun createNotification(): Notification {
        val intent = Intent(BoyiaConstants.BOYIA_MAIN_ACTION)
        val pendingIntent = if (Build.VERSION.SDK_INT >= 23) {
            PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_IMMUTABLE)
        } else {
            PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT)
        }

        return NotificationCompat.Builder(this, BOYIA_APP_CHANNEL_ID)
                .setAutoCancel(true)
                .setSmallIcon(R.drawable.ic_launcher)
                .setWhen(System.currentTimeMillis())
                .setContentIntent(pendingIntent)
                .build()
    }
}