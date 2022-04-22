package com.boyia.app.shell.model

import com.boyia.app.common.json.JsonAnnotation.JsonKey

class BoyiaAppItem {
    // 自动生成get set
    @field:JsonKey(name = "name") var name: String? = null
    var cover: String? = null
    var url: String? = null
    @field:JsonKey(name = "id") var appId: Int = 0
    var versionCode: Int = 0

    override fun toString() : String {
        return String.format(
                "BoyiaAppItem: name=%s, image=%s, url=%s appId=%d, versionCode=%d"
                , name, cover, url, appId, versionCode)
    }
}