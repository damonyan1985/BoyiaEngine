package com.boyia.app.shell.model

import com.boyia.app.common.json.JsonAnnotation.JsonKey

data class BoyiaAppItem(
    @field:JsonKey(name = "name") var name: String? = null,
    var cover: String? = null,
    var url: String? = null,
    @field:JsonKey(name = "id") var appId: Int = 0,
    var versionCode: Int = 0
)