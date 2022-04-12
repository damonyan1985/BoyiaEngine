package com.boyia.app.shell.api

import com.boyia.app.shell.model.BoyiaAppItem
import com.boyia.app.shell.model.BoyiaAppListModel

interface IBoyiaHomeLoader {
    fun loadAppList(callback: BoyiaAppListModel.LoadCallback)
    fun appListCount(): Int
    fun appItem(index: Int): BoyiaAppItem
}