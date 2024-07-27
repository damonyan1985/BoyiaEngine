package com.boyia.app.shell.util

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Build
import androidx.core.content.FileProvider
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import java.io.File

object BoyiaAppShare {
    const val TAG = "BoyiaShare"
    private const val INSTAGRAM_ACTION = "com.instagram.share.ADD_TO_STORY"
    private const val FACE_BOOK_ID = "source_application"
    private const val INSTAGRAM_PACKAGE_NAME = "com.instagram.android"
    private const val WHATS_UP_PACKAGE_NAME = "com.whatsapp"
    private const val SNAP_CHAT_PACAKGE_NAME = "com.snapchat.android"
    private const val SHARE_IMAGE_TYPE = "image/*"
    private const val SHARE_TEXT_TYPE = "text/plain"

    /**
     * 分享到Instagram
     */
    @JvmStatic
    fun shareInstagram(ctx: Activity?, path: String?, fid: String?) {
        BoyiaLog.d(TAG, "ctx is null: " + (ctx == null))

        val ctx = ctx ?: return
        if (!BoyiaUtils.isAppInstalled(ctx, INSTAGRAM_PACKAGE_NAME)) {
            BoyiaLog.d(TAG, "instagram is not install!")
            return
        }
        val path = path ?: return

        val intent = Intent(INSTAGRAM_ACTION)

        val uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            FileProvider.getUriForFile(ctx, "${ctx.packageName}.fileprovider", File(path))
        } else {
            Uri.fromFile(File(path))
        }

//        intent.putExtra("interactive_asset_uri", uri)
//        intent.putExtra("top_background_color", "#33FF33");
//        intent.putExtra("bottom_background_color", "#FF00FF");

        if (fid != null) {
            intent.putExtra(FACE_BOOK_ID, fid)
        }
        intent.setDataAndType(uri, SHARE_IMAGE_TYPE)
        intent.flags = Intent.FLAG_GRANT_READ_URI_PERMISSION

        ctx.grantUriPermission(
            INSTAGRAM_PACKAGE_NAME, uri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
        if (ctx.packageManager?.resolveActivity(intent, 0) != null) {
            BoyiaLog.d(TAG, "start Instagram")
            ctx.startActivityForResult(intent, 0);
        }
    }

    @JvmStatic
    fun shareInstagramContacts(ctx: Activity?, path: String?, msg: String?) {
        val ctx = ctx ?: return
        if (!BoyiaUtils.isAppInstalled(ctx, INSTAGRAM_PACKAGE_NAME)) {
            BoyiaLog.d(TAG, "instagram is not install!")
            return
        }
        val path = path ?: return

        val uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            FileProvider.getUriForFile(ctx, "${ctx.packageName}.fileprovider", File(path))
        } else {
            Uri.fromFile(File(path))
        }
        val shareIntent = Intent()
        shareIntent.action = Intent.ACTION_SEND
        shareIntent.setPackage(INSTAGRAM_PACKAGE_NAME)
        if (msg != null) {
            shareIntent.putExtra(Intent.EXTRA_TEXT, msg)
        }
        shareIntent.putExtra(Intent.EXTRA_STREAM, uri)
        shareIntent.type = SHARE_IMAGE_TYPE
        shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)

        ctx.startActivity(shareIntent)
    }

    /**
     * 分享whatsup
     * ctx 当前activity
     * path 获取的图片路径
     * msg 需要发送的消息
     */
    @JvmStatic
    fun shareWhatsup(ctx: Activity?, path: String?, msg: String?) {
        val ctx = ctx ?: return
        if (!BoyiaUtils.isAppInstalled(ctx, WHATS_UP_PACKAGE_NAME)) {
            BoyiaLog.d(TAG, "whatsup is not install!")
            return
        }

        if (path == null && msg == null) {
            return
        }

        val shareIntent = Intent()
        if (path != null) {
            val uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                FileProvider.getUriForFile(ctx, "${ctx.packageName}.fileprovider", File(path))
            } else {
                Uri.fromFile(File(path))
            }

            shareIntent.type = SHARE_IMAGE_TYPE
            shareIntent.putExtra(Intent.EXTRA_STREAM, uri)
        } else {
            shareIntent.type = SHARE_TEXT_TYPE
        }

        shareIntent.action = Intent.ACTION_SEND
        shareIntent.setPackage(WHATS_UP_PACKAGE_NAME)
        if (msg != null) {
            shareIntent.putExtra(Intent.EXTRA_TEXT, msg)
        }

        shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        ctx.startActivity(shareIntent)
    }

    /**
     * 分享snapchat
     */
    @JvmStatic
    fun shareSnapchat(ctx: Activity?, path: String?, msg: String?) {
        val ctx = ctx ?: return
        if (!BoyiaUtils.isAppInstalled(ctx, SNAP_CHAT_PACAKGE_NAME)) {
            BoyiaLog.d(TAG, "snapchat is not install!")
            return
        }

        if (path == null && msg == null) {
            return
        }

        val shareIntent = Intent()
        if (path != null) {
            val uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                FileProvider.getUriForFile(ctx, "${ctx.packageName}.fileprovider", File(path))
            } else {
                Uri.fromFile(File(path))
            }

            shareIntent.type = SHARE_IMAGE_TYPE
            shareIntent.putExtra(Intent.EXTRA_STREAM, uri)
        }

        shareIntent.action = Intent.ACTION_SEND
        shareIntent.setPackage(SNAP_CHAT_PACAKGE_NAME)
        if (msg != null) {
            shareIntent.putExtra(Intent.EXTRA_TEXT, msg)
        }
        shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)

        ctx.startActivity(shareIntent)
    }

    /**
     * 短信跳转
     */
    @JvmStatic
    fun shareToSMS(ctx: Activity?, text: String?, phone: String?) {
        val ctx = ctx ?: return
        val phone = if (BoyiaUtils.isTextEmpty(phone)) "" else text

        val intent = Intent(Intent.ACTION_SENDTO, Uri.parse("smsto: $phone"))
        intent.putExtra("sms_body", text ?: "")
        ctx.startActivity(intent)
    }
}