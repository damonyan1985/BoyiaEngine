package com.boyia.app.shell.util

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.Dp
import com.boyia.app.common.utils.BoyiaUtils

// 提供一个dp属性
val Int.dp: Int
    get() = BoyiaUtils.dp(this)

/**
 * 针对compose进行换算
 */
val Int.dpx: Dp
    get() = Dp(this.toFloat())

@Composable
fun dpx(value: Int): Dp {
    return with(LocalDensity.current) { value.dp.toDp() }
}