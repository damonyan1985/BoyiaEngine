package com.boyia.app.shell.model

import java.util.concurrent.CopyOnWriteArrayList

class BoyiaAppListModel: BoyiaAppModel() {
    companion object {
        const val TAG = "BoyiaAppListModel"
        //const val APP_LIST_URL = "http://47.98.206.177/test.json"
        const val APP_LIST_URL = "boyia_assets://boyia.json";
    }

    // 工作线程中添加数据，UI线程中读取数据
    // 该场景适合CopyOnWriteArrayList，刷新只会添加元素而不会删除元素
    val appList: CopyOnWriteArrayList<BoyiaAppItem> = CopyOnWriteArrayList()

    fun requestAppList(callback: LoadCallback) {
        requestImpl(APP_LIST_URL, object: ModelDataCallback<BoyiaAppListData> {
            override fun onLoadData(data: BoyiaAppListData) {
                if (data.apps == null || data.apps!!.isEmpty()) {
                    return
                }

                appList.addAll(data.apps!!)
                callback.onLoaded()
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