package com.boyia.app.shell.model

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.BoyiaLoader
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.Action
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.loader.mue.MueTask
import com.boyia.app.loader.mue.Subscriber
import java.io.ByteArrayOutputStream
import java.lang.StringBuilder
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener as SimpleLoaderListener

object BoyiaAppListModel {
    const val TAG = "BoyiaAppListModel"
    const val APP_LIST_URL = "http://47.98.206.177/test.json"

    fun requestAppList(callback: LoadCallback) {
        MueTask.create { subscriber: Subscriber<String> -> run {
                    var buffer = ByteArrayOutputStream()
                    BoyiaLoader(object : SimpleLoaderListener() {
                        override fun onLoadDataReceive(bytes: ByteArray?, length: Int, msg: Any?) {
                            buffer.write(bytes, 0, length)
                        }

                        override fun onLoadFinished(msg: Any?) {
                            //buffer.toString("UTF-8")
                            val json = buffer.toString("UTF-8")
                            BoyiaLog.d(TAG, "requestAppList json = $json")
                            subscriber.onNext(json)
                            subscriber.onComplete();
                        }

                        override fun onLoadError(msg: String?, p1: Any?) {
                            subscriber.onError(null)
                        }
                    }).load(APP_LIST_URL)
                } }
                .subscribeOn(JobScheduler.jobScheduler())
                .observeOn(MainScheduler.mainScheduler())
                .subscribe(object : Subscriber<String> {
                    override fun onNext(result: String?) {
                        callback.onResult(result)
                    }

                    override fun onError(error: Throwable?) {
                    }

                    override fun onComplete() {
                    }
                })
    }

    interface LoadCallback {
        fun onResult(result: String?)
    }
}