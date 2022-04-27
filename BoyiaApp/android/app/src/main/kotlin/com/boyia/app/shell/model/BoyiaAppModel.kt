package com.boyia.app.shell.model

import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.BoyiaLoader
import com.boyia.app.loader.http.HTTPFactory
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.loader.mue.MueTask
import com.boyia.app.loader.mue.Subscriber
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener
import java.io.ByteArrayOutputStream

open class BoyiaAppModel {
    companion object {
        const val TAG = "BoyiaAppModel"
    }

    /**
     * 抽象一个下载数据的方法
     */
    inline fun <reified T> requestImpl(url: String, cb: ModelDataCallback<T>) {
        MueTask.create { subscriber: Subscriber<T> -> run {
            val buffer = ByteArrayOutputStream()
            BoyiaLoader(object : BoyiaSimpleLoaderListener {
                override fun onLoadDataReceive(bytes: ByteArray?, length: Int, msg: Any?) {
                    buffer.write(bytes, 0, length)
                }

                override fun onLoadFinished(msg: Any?) {
                    val json = buffer.toString(HTTPFactory.HTTP_CHARSET_UTF8)
                    BoyiaLog.d(TAG, "requestAppList json = $json")
                    val data: T = BoyiaJson.jsonParse(json, T::class.java)
                    subscriber.onNext(data)
                    subscriber.onComplete()
                }

                override fun onLoadError(msg: String?, p1: Any?) {
                    subscriber.onError(null)
                }
            }).load(url)
        } }
        .subscribeOn(JobScheduler.jobScheduler())
        //.observeOn(MainScheduler.mainScheduler())
        .subscribe(object : Subscriber<T> {
            override fun onNext(result: T) {
                cb.onLoadData(result)
            }

            override fun onError(error: Throwable?) {
            }

            override fun onComplete() {
                BoyiaLog.d(TAG, "Model data load complete")
            }
        })
    }

    interface ModelDataCallback<T> {
        fun onLoadData(data: T)
    }
}