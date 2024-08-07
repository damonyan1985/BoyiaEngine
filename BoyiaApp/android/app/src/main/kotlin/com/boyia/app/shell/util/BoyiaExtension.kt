package com.boyia.app.shell.util

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.sp
import com.boyia.app.common.utils.BoyiaUtils

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
val Int.bpx: Dp
    @Composable
    get() = dpx(this)

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

/**
 * px转sp
 */
fun bsp(value: Int): TextUnit {
    return BoyiaUtils.px2sp(value.toFloat()).sp
}

/**
 * fontsize px to sp
 */
val Int.bsp: TextUnit
    get() = bsp(this)

val Int.sp: Float
    get() = BoyiaUtils.px2sp(this.toFloat())