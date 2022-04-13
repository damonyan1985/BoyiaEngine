package com.boyia.app.shell.model

class BoyiaAppItem {
    // 自动生成get set
    var name: String? = null
    var cover: String? = null
    var url: String? = null
    var appId: Int = 0
    var versionCode: Int = 0

    override fun toString() : String {
        return String.format(
                "BoyiaAppItem: name=%s, image=%s, url=%s appId=%d, versionCode=%d"
                , name, cover, url, appId, versionCode)
    }
}