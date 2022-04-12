package com.boyia.app.shell.model

import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.BoyiaLoader
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.Action
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.loader.mue.MueTask
import com.boyia.app.loader.mue.Subscriber
import java.io.ByteArrayOutputStream
import java.lang.StringBuilder
import java.util.ArrayList
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener as SimpleLoaderListener

class BoyiaAppListModel {
    companion object {
        const val TAG = "BoyiaAppListModel"
        const val APP_LIST_URL = "http://47.98.206.177/test.json"
    }

    var appList: ArrayList<BoyiaAppItem> = ArrayList()

    fun requestAppList(callback: LoadCallback) {
        MueTask.create { subscriber: Subscriber<BoyiaAppListData> -> run {
                    var buffer = ByteArrayOutputStream()
                    BoyiaLoader(object : SimpleLoaderListener() {
                        override fun onLoadDataReceive(bytes: ByteArray?, length: Int, msg: Any?) {
                            buffer.write(bytes, 0, length)
                        }

                        override fun onLoadFinished(msg: Any?) {
                            //buffer.toString("UTF-8")
                            val json = buffer.toString("UTF-8")
                            BoyiaLog.d(TAG, "requestAppList json = $json")
                            var data: BoyiaAppListData = BoyiaJson.jsonParse(json, BoyiaAppListData::class.java)
                            subscriber.onNext(data)
                            subscriber.onComplete();
                        }

                        override fun onLoadError(msg: String?, p1: Any?) {
                            subscriber.onError(null)
                        }
                    }).load(APP_LIST_URL)
                } }
                .subscribeOn(JobScheduler.jobScheduler())
                .observeOn(MainScheduler.mainScheduler())
                .subscribe(object : Subscriber<BoyiaAppListData> {
                    override fun onNext(result: BoyiaAppListData) {
                        //callback.onResult(result)
                        appList.addAll(result.apps)
                        // 通知UI层刷新
                        callback.onLoaded()
                    }

                    override fun onError(error: Throwable?) {
                    }

                    override fun onComplete() {
                    }
                })
    }



    interface LoadCallback {
        fun onLoaded()
    }
}