package com.boyia.app.shell.home

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import android.view.View
import com.boyia.app.common.BaseApplication
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener
import com.boyia.app.shell.update.Downloader

@SuppressLint("ClickableViewAccessibility")
class BoyiaDownloadMask(context: Context, private val downloadCallback: DownloadCallback): View(context), BoyiaSimpleLoaderListener {
    companion object {
        const val TAG = "BoyiaDownloadMask"
        const val TEST_URL = "https://klxxcdn.oss-cn-hangzhou.aliyuncs.com/histudy/hrm/media/bg3.mp4"
    }

    private var progress: Float = 0F
    private var fileTotalSize: Long = 0L
    private var fileCurrentSize: Int = 0

    init {
        // 禁止点击事件透传
        setOnTouchListener { _, _ ->
            // TODO 启动下载任务
            Downloader(this).download(TEST_URL)
            return@setOnTouchListener true
        }
    }

    fun setProgress(prog: Float) {
        progress = prog
        invalidate()
    }

    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)
        if (progress == 1F) {
            return
        }

        val paint = Paint()
        paint.color = 0x88000000.toInt()

        if (progress == 0F) {
            canvas?.drawRect(Rect(0, 0, width,height), paint)
            return
        }

        val path = getPath() ?: return;
        canvas?.drawPath(path, paint)
    }

    private fun getPath(): Path? {
        val w = width.toFloat()
        val h = height.toFloat()
        val path = Path()
        path.moveTo(w / 2, 0F)
        path.lineTo(w / 2, h / 2)
        if (progress <= 0.125) {
            val x1 = w/2 + (w/2) * (progress / 0.125)
            if (progress < 0.125) {
                path.lineTo(x1.toFloat(), 0F)
            }
            path.lineTo(w, 0F)
            path.lineTo(w, h)
            path.lineTo(0F, h)
            path.lineTo(0F, 0F)
            path.moveTo(w / 2, 0F)
        } else if (progress > 0.125 && progress <= 0.375) {
            val y1 = ((progress - 0.125) / 0.25) * h
            if (progress < 0.375) {
                path.lineTo(w, y1.toFloat())
            }
            path.lineTo(w, h)
            path.lineTo(0F, h)
            path.lineTo(0F, 0F)
            path.moveTo(w / 2, 0F)
        } else if (progress > 0.375 && progress <= 0.625) {
            val x1 = w - ((progress - 0.375) / 0.25) * w
            if (progress < 0.625) {
                path.lineTo(x1.toFloat(), h)
            }
            path.lineTo(0F, h)
            path.lineTo(0F, 0F)
            path.moveTo(w / 2, 0F)

        } else if (progress > 0.625 && progress <= 0.875) {
            val y1 = h - ((progress - 0.625) / 0.25) * h
            if (progress < 0.875) {
                path.lineTo(0F, y1.toFloat())
            }
            path.lineTo(0F, 0F)
            path.moveTo(w / 2, 0F)
        } else if (progress > 0.875 && progress < 1.0) {
            val x1 = ((progress - 0.875) / 0.25) * w
            path.lineTo(x1.toFloat(), 0F)
            path.moveTo(w / 2, 0F)
        }

        return path
    }

    override fun onLoadDataSize(size: Long) {
        fileTotalSize = size
    }

    override fun onLoadDataReceive(p0: ByteArray?, length: Int, p2: Any?) {
        fileCurrentSize += length
        if (fileTotalSize == 0L) {
            return
        }

        val progress = fileCurrentSize.toFloat() / fileTotalSize.toFloat()
        //BoyiaLog.d(TAG, "BoyiaDownloadMask onLoadDataReceive progress=" + (progress * 100) + "%");

        MainScheduler.mainScheduler().sendJob {
            setProgress(progress)
        }
    }

    override fun onLoadFinished(p0: Any?) {
        BoyiaLog.d(TAG, "BoyiaDownloadMask onLoadFinished")
        downloadCallback.onCompleted()
    }

    override fun onLoadError(p0: String?, p1: Any?) {
    }

    interface DownloadCallback {
        fun onCompleted()
    }
}