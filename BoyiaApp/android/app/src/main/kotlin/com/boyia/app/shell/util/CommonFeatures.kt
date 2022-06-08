package com.boyia.app.shell.util

import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Build
import androidx.core.app.NotificationCompat
import androidx.core.graphics.drawable.IconCompat
import com.boyia.app.common.BaseApplication
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.loader.image.IBoyiaImage
import com.boyia.app.shell.R
import com.boyia.app.shell.service.BoyiaNotifyService

// 共用功能
object CommonFeatures {
    /**
     * 发送推送信息
     */
    fun sendNotification(action: String, title: String, icon: String) {
        val context = BaseApplication.getInstance()
        val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        val intent = Intent(action)
        val pendingIntent = if (Build.VERSION.SDK_INT >= 23) {
            PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_IMMUTABLE)
        } else {
            PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT)
        }

        BoyiaImager.loadImage(icon, object : IBoyiaImage {
            override fun setImageURL(url: String?) {
            }

            override fun getImageURL(): String {
                return icon
            }

            override fun setImage(bitmap: Bitmap?) {
                val notification = NotificationCompat.Builder(context, BoyiaNotifyService.BOYIA_APP_CHANNEL_ID)
                        .setAutoCancel(true)
                        .setSmallIcon(R.drawable.ic_launcher)
                        .setLargeIcon(bitmap)
                        //.setWhen(System.currentTimeMillis())
                        .setContentIntent(pendingIntent)
                        .setContentTitle(title)
                        .build()

                manager.notify(0, notification)
            }

            override fun getImageWidth(): Int {
                return 100.dp
            }

            override fun getImageHeight(): Int {
                return 100.dp
            }
        })
    }
}