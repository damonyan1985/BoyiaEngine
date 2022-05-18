package com.boyia.app.shell.setting

import com.boyia.app.shell.module.IModule
import com.boyia.app.shell.module.IModuleContext

class BoyiaSettingModule : IModule {
    override fun init() {

    }

    override fun show(context: IModuleContext) {
        val fragment = BoyiaSettingFragment()

        val fragmentTransaction = context.getActivity().supportFragmentManager.beginTransaction()
        fragmentTransaction.add(context.rootId(), fragment)
        fragmentTransaction.commit()
    }

    override fun dispose() {
    }
}