package com.boyia.app.shell.model

data class BoyiaUserData(
    var data: BoyiaUserInfo? = null,
    var userToken: String? = null
) : BoyiaBaseData()