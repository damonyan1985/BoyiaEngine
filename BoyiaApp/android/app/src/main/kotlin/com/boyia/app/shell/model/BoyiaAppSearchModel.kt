package com.boyia.app.shell.model

import androidx.compose.runtime.mutableStateListOf
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.http.HTTPFactory
import java.util.concurrent.CopyOnWriteArrayList

class BoyiaAppSearchModel {
    //private var appList = MutableLiveData(CopyOnWriteArrayList<BoyiaAppItem>())
    var appList = mutableStateListOf<BoyiaAppItem>()

    fun appendAppList(key: String) {
        loadAppList(key, appList.size, 20) {
            appList.addAll(it)
        }
    }

    // 点击搜索按钮响应
    fun searchAppList(key: String) {
        loadAppList(key, 0, 20) {
            appList.clear()
            appList.addAll(it)
        }
    }

    private fun loadAppList(key: String, pos: Int, size: Int, loadCB: (list: ArrayList<BoyiaAppItem>) -> Unit) {
        if (BoyiaUtils.isTextEmpty(key)) {
            return
        }

        BoyiaModelUtil.request(BoyiaModelUtil.SEARCH_APP_LIST_URL,
                object: BoyiaModelUtil.ModelDataCallback<BoyiaAppListData> {
                    override fun onLoadData(data: BoyiaAppListData) {
                        BoyiaLog.d(BoyiaAppListModel.TAG, "BoyiaAppListData retMsg = ${data.retMsg}")
                        if (data.data == null || data.data!!.isEmpty()) {
                            return
                        }

                        loadCB(data.data!!)
                    }
                },
                HTTPFactory.HTTP_POST_METHOD,
                mapOf(HTTPFactory.HeaderKeys.CONTENT_TYPE to HTTPFactory.HeaderValues.FORM, "User-Token" to "none"),
                "key=${key}&pos=${pos}&size=${size}"
        )
    }
}