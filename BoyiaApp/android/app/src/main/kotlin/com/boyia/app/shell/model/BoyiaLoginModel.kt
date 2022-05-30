package com.boyia.app.shell.model

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.http.HTTPFactory

class BoyiaLoginModel {
    companion object {
        const val TAG = "BoyiaLoginModel"
    }

    fun login(name: String?, password: String?, callback: (info: BoyiaUserInfo) -> Unit) {
        if (BoyiaUtils.isTextEmpty(name)
                || BoyiaUtils.isTextEmpty(password)
        ) {
            return
        }

        BoyiaModelUtil.request(
                BoyiaModelUtil.LOGIN_URL,
                object: BoyiaModelUtil.ModelDataCallback<BoyiaUserData> {
                    override fun onLoadData(data: BoyiaUserData) {
                        BoyiaLog.d(TAG, "retCode = " + data.retCode)
                        if (data.retCode != 200) {
                            return
                        }

                        data.data?.let { callback(it) }
                    }
                },
                method = HTTPFactory.HTTP_POST_METHOD,
                headers = mapOf("Content-Type" to "application/x-www-form-urlencoded", "User-Token" to "none"),
                data = "name=${name}&pwd=${password}"
        )
    }
}