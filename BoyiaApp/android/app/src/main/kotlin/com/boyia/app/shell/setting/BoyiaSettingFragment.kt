package com.boyia.app.shell.setting

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.RelativeLayout
import androidx.fragment.app.Fragment
import com.boyia.app.common.utils.BoyiaUtils.dp

class BoyiaSettingFragment : Fragment() {
    private val SETTING_WIDTH = dp(200)

    private var rootLayout: RelativeLayout? = null

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout
        }

        rootLayout = RelativeLayout(context)
        val lp = ViewGroup.LayoutParams(
            SETTING_WIDTH,
            ViewGroup.LayoutParams.MATCH_PARENT
        )



        //rootLayout?.x = -1 * SETTING_WIDTH.toFloat()
        rootLayout?.layoutParams = lp
        rootLayout?.setBackgroundColor(0xFFEDEDED.toInt())
        return rootLayout
    }
}