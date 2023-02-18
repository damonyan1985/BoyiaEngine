package com.boyia.app.shell.model

import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.BoyiaLoader
import com.boyia.app.loader.http.HTTPFactory
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.loader.mue.MueFunction
import com.boyia.app.loader.mue.MueTask
import com.boyia.app.loader.mue.Subscriber
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener
import java.io.ByteArrayOutputStream

object BoyiaModelUtil {
    const val TAG = "BoyiaModelUtil"
    private const val HTTP_DOMAIN = "https://47.98.206.177:8443"
    //private const val HTTP_DOMAIN = "https://192.168.121.103:8443"
    private const val API_VERSION = "v1"

    const val LOGIN_URL = "${HTTP_DOMAIN}/user/${API_VERSION}/login"
    const val LOGOUT_URL = "${HTTP_DOMAIN}/user/${API_VERSION}/logout"
    const val APP_LIST_URL = "${HTTP_DOMAIN}/app/${API_VERSION}/appList"
    const val UPLOAD_URL = "${HTTP_DOMAIN}/file/${API_VERSION}/upload_user_icon"
    const val UPDATE_USER_URL = "${HTTP_DOMAIN}/user/${API_VERSION}/updateAdmin"
    const val SEARCH_APP_LIST_URL = "${HTTP_DOMAIN}/app/${API_VERSION}/search"

    fun getRemoteUrl(url: String?): String? {
        if (BoyiaUtils.isTextEmpty(url)) {
            return null
        }

        if (url?.startsWith("http")?.or(url.startsWith("https")) == true) {
            return url
        }

        return "$HTTP_DOMAIN$url"
    }

    fun getImageUrlWithToken(url: String?): String? {
        val imageUrl = getRemoteUrl(url) ?: return null
        return "$imageUrl?uid=${BoyiaLoginInfo.instance().user?.id}&token=${BoyiaUtils.getStringMD5(BoyiaLoginInfo.instance().token)}"
    }

    // 使用reified修饰的T可以作为java类使用
    // 例如可以使用T::class.java
    inline fun <reified T> request(
            url: String,
            cb: ModelDataCallback<T>? = null,
            method: Int = HTTPFactory.HTTP_GET_METHOD,
            headers: Map<String, String>? = null,
            data: String? = null,
            upload: Boolean = false
    ) {

        MueTask.create { subscriber: Subscriber<T> ->
            run {
                val buffer = ByteArrayOutputStream()
                val loader = BoyiaLoader(object : BoyiaSimpleLoaderListener {
                    override fun onLoadDataReceive(bytes: ByteArray?, length: Int, msg: Any?) {
                        if (bytes != null) {
                            buffer.write(bytes, 0, length)
                        }
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

                    override fun onUploadProgress(current: Long, total: Long, msg: Any?) {
                        super.onUploadProgress(current, total, msg)
                    }
                })

                loader.setRequestMethod(method)
                if (data != null) {
                    loader.setPostParam(data)
                }

                // 设置http头
                headers?.forEach {
                    loader.putRequestHeader(it.key, it.value)
                }

                if (upload) {
                    loader.upload(url, false, null)
                } else {
                    loader.load(url)
                }
            }
        }
        .subscribeOn(JobScheduler.jobScheduler())
        .observeOn(MainScheduler.mainScheduler())
        .subscribe(object : Subscriber<T> {
            override fun onNext(result: T) {
                cb?.onLoadData(result)
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