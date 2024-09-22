package com.boyia.app.shell.model

import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.http.HTTPFactory
import java.util.concurrent.CopyOnWriteArrayList
import com.boyia.app.shell.model.BoyiaModelUtil.ModelDataCallback
import com.boyia.app.shell.model.BoyiaModelUtil.SEARCH_APP_LIST_URL

class BoyiaAppListModel {
    companion object {
        const val TAG = "BoyiaAppListModel"
        //const val APP_LIST_URL = "http://192.168.2.5/test.json"
        const val APP_LIST_URL = "boyia_assets://boyia.json";
    }

    // 工作线程中添加数据，UI线程中读取数据
    // 该场景适合CopyOnWriteArrayList，刷新只会添加元素而不会删除元素
    // 读取时最好将array拿出来，即使用getArray获取引用
    val appList: CopyOnWriteArrayList<BoyiaAppItem> = CopyOnWriteArrayList()

    fun requestAppList(callback: LoadCallback) {
        if (appList.isNotEmpty()) {
            return
        }
        BoyiaModelUtil.request(APP_LIST_URL, object: ModelDataCallback<BoyiaAppListData> {
            override fun onLoadData(data: BoyiaAppListData) {
                BoyiaLog.d(TAG, "BoyiaAppListData retMsg = ${data.retMsg}")
                data.data?.let {
                    appList.addAll(data.data)
                    callback.onLoaded()
                }
            }
        })
    }

    fun clear() {
        appList.clear()
    }

    interface LoadCallback {
        fun onLoaded()
    }
}