package com.boyia.app.shell.model

import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.http.HTTPFactory

object BoyiaLoginModel {
    const val TAG = "BoyiaLoginModel"

    fun login(name: String?, password: String?, callback: (info: BoyiaUserInfo) -> Unit) {
        if (BoyiaUtils.isTextEmpty(name)
                || BoyiaUtils.isTextEmpty(password)
        ) {
            BoyiaLog.d(TAG, "name or passsword is emprt")
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

                        BoyiaLoginInfo.instance().token = data.userToken
                        BoyiaLoginInfo.instance().user = data.data

                        data.data?.let { callback(it) }
                    }
                },
                method = HTTPFactory.HTTP_POST_METHOD,
                headers = mapOf(HTTPFactory.HeaderKeys.CONTENT_TYPE to HTTPFactory.HeaderValues.FORM, "User-Token" to "none"),
                data = "name=${name}&pwd=${password}"
        )
    }

    fun update() {
        val info =  BoyiaJson.toJson(BoyiaLoginInfo.instance().user)
        BoyiaModelUtil.request<BoyiaBaseData>(
                BoyiaModelUtil.UPDATE_USER_URL,
                method = HTTPFactory.HTTP_POST_METHOD,
                headers = mapOf(
                        "User-Token" to (BoyiaLoginInfo.instance().token ?: "none"),
                        HTTPFactory.HeaderKeys.CONTENT_TYPE to HTTPFactory.HeaderValues.JSON
                ),
                data = info
        )
    }

    fun logout() {
        BoyiaModelUtil.request<BoyiaBaseData>(
                BoyiaModelUtil.LOGOUT_URL,
                method = HTTPFactory.HTTP_POST_METHOD,
                headers = mapOf("User-Token" to (BoyiaLoginInfo.instance().token ?: "none"))
        )
    }
}