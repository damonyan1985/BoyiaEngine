package com.boyia.app.shell.util

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.Dp
import com.boyia.app.common.utils.BoyiaUtils
import java.nio.channels.FileLock

/**
 * boyia dp扩展属性
 */
val Int.dp: Int
    get() = BoyiaUtils.dp(this)

/**
 * 防止与compose dp冲突
 */
val Int.dpx: Int
    get() = BoyiaUtils.dp(this)

val Int.dpf: Float
    get() = BoyiaUtils.dp(this).toFloat()
/**
 * boyia dp转compose dp
 */
@Composable
fun dpx(value: Int): Dp {
    return with(LocalDensity.current) { value.dp.toDp() }
}

/**
 * px转compose dp
 */
@Composable
fun toDp(value: Int): Dp {
    return with(LocalDensity.current) { value.toDp() }
}