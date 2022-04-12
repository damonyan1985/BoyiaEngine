package com.boyia.app.shell.home

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.util.AttributeSet
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.TextView
import com.boyia.app.common.utils.BoyiaUtils.dp
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.loader.image.BoyiaImageView

class BoyiaAppItemView(context: Context?, attrs: AttributeSet?) : LinearLayout(context, attrs) {
    var appIconView: BoyiaImageView? = null
    var appNameView: TextView? = null

    init {
        orientation = LinearLayout.VERTICAL
        appIconView = BoyiaImageView(context)
        var param: ViewGroup.LayoutParams = ViewGroup.LayoutParams(
                dp(224), dp(224)
        )

        addView(appIconView, param)

        appNameView = TextView(context)
        addView(appNameView)
        appNameView?.setTextColor(Color.BLACK)
        appNameView?.setBackgroundColor(Color.WHITE)

        setBackgroundColor(Color.WHITE)

        setOnClickListener {
            startApp("")
        }
    }

    fun startApp(info: String) {
//        val intent = Intent();
//        intent.action = "com.boyia.app.suba.action"
//        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
//        context.startActivity(intent)
        BoyiaAppLauncher.launch(null);
    }

    constructor(context: Context?) : this(context, null)
}