package com.boyia.app.shell.model

import com.boyia.app.common.json.JsonAnnotation.JsonKey

// data class对打印数据友好
data class BoyiaAppItem(
    @field:JsonKey(name = "name") var name: String? = null,
    var cover: String? = null,
    var url: String? = null,
    @field:JsonKey(name = "id") var appId: Int = 0,
    var versionCode: Int = 0
)