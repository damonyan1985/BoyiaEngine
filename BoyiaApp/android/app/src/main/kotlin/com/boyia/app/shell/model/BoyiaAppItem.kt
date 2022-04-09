package com.boyia.app.shell.model

class BoyiaAppItem {
    // 自动生成get set
    var name: String? = null
    var cover: String? = null

    override fun toString() : String {
        return String.format("BoyiaAppItem: name=%s and image=%s", name, cover)
    }
}