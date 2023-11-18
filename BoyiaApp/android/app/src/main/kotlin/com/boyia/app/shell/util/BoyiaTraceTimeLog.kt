package com.boyia.app.shell.util

import com.boyia.app.common.utils.BoyiaLog

object BoyiaTraceTimeLog {
    private const val TAG = "BoyiaTrace"

    /**
     * timeLocalMap key: 唯一标识，value: 时间
     */
    private val timeLocalMap = ThreadLocal<HashMap<Int, Long>>()
    private const val LINE = "══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════"

    @JvmStatic
    fun enter(key: Int) {
        var map = timeLocalMap.get()
        if (map == null) {
            map = HashMap()
            timeLocalMap.set(map)
        }

        map[key] = System.currentTimeMillis()
    }

    @JvmStatic
    fun exit(key: Int, className: String, methodName: String, argsType: String, returnType: String) {
        var map = timeLocalMap.get()
        val deltaTime = System.currentTimeMillis() - (map?.get(key) ?: 0)
        val builder = StringBuilder()
                .append(" ")
                .append("\n╔").append(LINE)
                .append("\n║ [Thread]:").append(Thread.currentThread().name)
                .append("\n║ [Class]:").append(className)
                .append("\n║ [Method]:").append(methodName)
                .append("\n║ [ArgsType]:").append(argsType)
                .append("\n║ [ReturnType]:").append(returnType)
                .append("\n║ [Time]:").append(deltaTime).append(" ms")
                .append("\n╚").append(LINE)

        BoyiaLog.i(TAG, builder.toString())
        map.remove(key)
    }
}