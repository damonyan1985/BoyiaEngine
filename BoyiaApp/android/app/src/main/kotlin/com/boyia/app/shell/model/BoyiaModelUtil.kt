package com.boyia.app.shell.model

import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.BoyiaLoader
import com.boyia.app.loader.http.HTTPFactory
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.MueTask
import com.boyia.app.loader.mue.Subscriber
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener
import java.io.ByteArrayOutputStream

object BoyiaModelUtil {
    const val TAG = "BoyiaModelUtil"
    const val HTTP_DOMAIN = "https://127.0.0.1:8443/"
    const val API_VERSION = "v1"

    const val LOGIN_URL = "${HTTP_DOMAIN}user/${API_VERSION}/login"
    const val LOGOUT_URL = "${HTTP_DOMAIN}user/${API_VERSION}/logout"
    const val APP_LIST_URL = "${HTTP_DOMAIN}app/${API_VERSION}/appList"

    inline fun <reified T> request(
            url: String,
            cb: ModelDataCallback<T>,
            method: Int = HTTPFactory.HTTP_GET_METHOD,
            headers: Map<String, String>? = null,
            data: String? = null) {

        MueTask.create { subscriber: Subscriber<T> -> run {
            val buffer = ByteArrayOutputStream()
            val loader = BoyiaLoader(object : BoyiaSimpleLoaderListener {
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
            })

            loader.setRequestMethod(method)
            if (data != null) {
                loader.setPostParam(data)
            }

            headers?.forEach {
                loader.putRequestHeader(it.key, it.value)
            }

            headers?.forEach {
                loader.putRequestHeader(it.key, it.value)
            }

            loader.load(url)
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