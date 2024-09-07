package com.boyia.app.shell.home

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.BoyiaBridge
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.update.Downloader
import com.boyia.app.shell.update.Downloader.DownLoadProgressListener

@SuppressLint("ClickableViewAccessibility")
class BoyiaDownloadMask(url: String, context: Context, private val downloadCallback: DownloadCallback): View(context), DownLoadProgressListener {
    companion object {
        const val TAG = "BoyiaDownloadMask"
        const val TEST_URL = "https://klxxcdn.oss-cn-hangzhou.aliyuncs.com/histudy/hrm/media/bg2.mp4?id=5"
    }

    private var progress: Float = 0F
    private var isDownloading = false

    init {
        // 禁止点击事件透传
        setOnTouchListener { _, e ->
            if (isDownloading) {
                return@setOnTouchListener true
            }
            // TODO 启动下载任务
            if (e.action == MotionEvent.ACTION_DOWN) {
                Downloader(this).download(url)
                isDownloading = true
            }
            return@setOnTouchListener true
        }
    }

    private fun setProgress(progress: Float) {
        this.progress = progress
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (progress == 1F) {
            return
        }

        val paint = Paint()
        paint.color = 0x88000000.toInt()

        if (progress == 0F) {
            canvas?.drawRect(Rect(0, 0, width, height), paint)
            return
        }

        val path = getPath() ?: return
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
            //path.moveTo(w / 2, 0F)
        } else if (progress > 0.125 && progress <= 0.375) {
            val y1 = ((progress - 0.125) / 0.25) * h
            if (progress < 0.375) {
                path.lineTo(w, y1.toFloat())
            }
            path.lineTo(w, h)
            path.lineTo(0F, h)
            path.lineTo(0F, 0F)
            //path.moveTo(w / 2, 0F)
        } else if (progress > 0.375 && progress <= 0.625) {
            val x1 = w - ((progress - 0.375) / 0.25) * w
            if (progress < 0.625) {
                path.lineTo(x1.toFloat(), h)
            }
            path.lineTo(0F, h)
            path.lineTo(0F, 0F)
            //path.moveTo(w / 2, 0F)

        } else if (progress > 0.625 && progress <= 0.875) {
            val y1 = h - ((progress - 0.625) / 0.25) * h
            if (progress < 0.875) {
                path.lineTo(0F, y1.toFloat())
            }
            path.lineTo(0F, 0F)
            //path.moveTo(w / 2, 0F)
        } else if (progress > 0.875 && progress < 1.0) {
            val x1 = ((progress - 0.875) / 0.25) * w
            path.lineTo(x1.toFloat(), 0F)
            //path.moveTo(w / 2, 0F)
        }

        path.close()
        return path
    }

    override fun onProgress(current: Long, size: Long) {
        if (size == 0L) {
            return
        }

        //BoyiaLog.d(TAG, "BoyiaDownloadMask fileCurrentSize=$current, fileTotalSize=$size")

        val progress = current.toFloat() / size.toFloat()
        //BoyiaLog.d(TAG, "BoyiaDownloadMask onLoadDataReceive progress=" + (progress * 100) + "%");

        MainScheduler.mainScheduler().sendJob {
            setProgress(progress)
        }
    }

    override fun onCompleted() {
        BoyiaLog.d(TAG, "BoyiaDownloadMask onLoadFinished")
        downloadCallback.onCompleted()
    }

    interface DownloadCallback {
        fun onCompleted()
    }
}