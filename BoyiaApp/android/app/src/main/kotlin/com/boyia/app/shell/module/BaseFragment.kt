package com.boyia.app.shell.module

import android.os.Bundle
import androidx.fragment.app.Fragment

open class BaseFragment: Fragment() {
    open fun canPop() : Boolean {
        return true
    }

    open fun hide() {}

    open fun customTag() : String = ""
}