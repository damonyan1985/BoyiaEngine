package com.boyia.app.shell.module

import android.view.animation.Animation
import android.view.animation.AnimationUtils
import androidx.fragment.app.FragmentTransaction
import com.boyia.app.shell.R

open class NavigationFragment: BaseFragment() {
    override fun onCreateAnimation(transit: Int, enter: Boolean, nextAnim: Int): Animation? {
        if (transit == FragmentTransaction.TRANSIT_FRAGMENT_OPEN) {
            if (enter) {
                // 表示普通的进入的动作，比如add、show、attach等
                return AnimationUtils.loadAnimation(context, R.anim.page_in);
            }
        } else if (transit == FragmentTransaction.TRANSIT_FRAGMENT_CLOSE) {
            if (!enter) {
                // 表示一个退出动作，比如出栈、remove、hide、detach等
                return AnimationUtils.loadAnimation(context, R.anim.page_out);
            }
        }
        return null;
    }

    override fun hide() {
        val fm = activity?.supportFragmentManager
        fm?.popBackStack()
    }
}