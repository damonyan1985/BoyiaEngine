package com.boyia.app.shell.module

import java.util.concurrent.ConcurrentHashMap

class ModuleManager {
    companion object {
        const val LOGIN = "login"
        const val HOME = "home"

        fun instance() = Holder.manager
    }

    private object Holder {
        val manager = ModuleManager()
    }

    private val moduleMap: ConcurrentHashMap<String, IModule> = ConcurrentHashMap()

    fun register(key: String, module: IModule) {
        moduleMap[key] = module
        module.init()
    }

    fun getModule(key: String): IModule? {
        return moduleMap[key]
    }
}