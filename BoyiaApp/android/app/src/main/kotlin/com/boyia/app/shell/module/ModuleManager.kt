package com.boyia.app.shell.module

import java.util.concurrent.ConcurrentHashMap

class ModuleManager {
    companion object {
        const val LOGIN = "login"
        const val HOME = "home"
        const val IPC = "ipc"
        const val SETTING = "setting"

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

    fun remove(key: String) {
        val module = moduleMap.remove(key)
        module?.dispose()
    }

    fun hide() {
        moduleMap.forEach { it
            if (it.value is IUIModule) {
                it.value.hide()
            }
        }
    }

    fun dispose() {
        moduleMap.forEach { it
            it.value.dispose()
        }
    }
}