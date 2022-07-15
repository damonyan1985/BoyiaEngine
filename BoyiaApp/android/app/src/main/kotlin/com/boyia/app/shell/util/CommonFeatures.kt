package com.boyia.app.shell.util

import android.app.Activity
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.Outline
import android.net.Uri
import android.os.Build
import android.provider.MediaStore
import android.view.View
import android.view.ViewGroup
import android.view.ViewOutlineProvider
import android.widget.LinearLayout
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContract
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.app.NotificationCompat
import com.boyia.app.common.BaseApplication
import com.boyia.app.common.utils.BoyiaFileUtil
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.loader.image.BoyiaImageView
import com.boyia.app.loader.image.BoyiaImager
import com.boyia.app.loader.image.IBoyiaImage
import com.boyia.app.shell.R
import com.boyia.app.shell.model.BoyiaLoginInfo
import com.boyia.app.shell.model.BoyiaModelUtil
import com.boyia.app.shell.service.BoyiaNotifyService

// 共用功能
object CommonFeatures {
    const val TAG = "CommonFeatures"
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

    // 为view设置圆角
    fun setViewRadius(view: View, radius: Int) {
        view.outlineProvider = object : ViewOutlineProvider() {
            override fun getOutline(view: View, outline: Outline?) {
                outline?.setRoundRect(0, 0, view.width, view.height, radius.toFloat())
            }
        }

        view.clipToOutline = true
    }

    fun registerPickerImage(context: AppCompatActivity, pickerCB: (path: String) -> Unit): ActivityResultLauncher<Unit?> {
        return context.registerForActivityResult(object: ActivityResultContract<Unit?, Uri?>() {
            override fun createIntent(context: Context, input: Unit?): Intent {
                return Intent(Intent.ACTION_PICK).setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*")
            }

            override fun parseResult(resultCode: Int, intent: Intent?): Uri? {
                return intent?.data
            }
        }) { uri: Uri? ->
            if (uri != null) {
                pickerCB(BoyiaFileUtil.getRealFilePath(context, uri))
            }
        }
    }

    fun getFragmentHeight(activity: Activity) : Int {
        return BoyiaUtils.getRealScreenSize().y - BoyiaUtils.getNavigationBarHeight(activity)
    }

    /**
     * 在compose中使用BoyiaImageView
     */
    @Composable
    fun ComposeBoyiaImageView(width: Int, height: Int, radius: Int, url: String?) {
        AndroidView(factory = { ctx ->
            BoyiaImageView(ctx, radius).apply {
                layoutParams = LinearLayout.LayoutParams(width, height)
                load(BoyiaModelUtil.getImageUrlWithToken(url))
            }
        })
    }

    @Composable
    fun marginTop(top: Int) {
        Spacer(modifier = Modifier.height(dpx(value = top)))
    }

    @Composable
    fun marginLeft(left: Int) {
        Spacer(modifier = Modifier.width(dpx(value = left)))
    }
}