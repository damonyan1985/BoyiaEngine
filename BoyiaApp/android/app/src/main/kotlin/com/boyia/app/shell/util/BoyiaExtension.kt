package com.boyia.app.shell.util

import com.boyia.app.common.utils.BoyiaUtils

// 提供一个dp属性
val Int.dp: Int
    get() = BoyiaUtils.dp(this)
