package com.boyia.app.shell.setting

import android.animation.Animator
import android.animation.ValueAnimator
import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.LinearInterpolator
import android.widget.Button
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.loader.image.BoyiaImageView
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideListener
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideCallback
import com.boyia.app.shell.util.dp

class BoyiaSettingFragment(private val module: BoyiaSettingModule) : BaseFragment() {
    companion object {
        const val TAG = "BoyiaSettingFragment"
    }

    private val SETTING_WIDTH = 320.dp

    private var rootLayout: RelativeLayout? = null
    private var animator: ValueAnimator? = null
    private var listener: SlideListener? = null

    fun setSlideListener(listener: SlideListener) {
        this.listener = listener
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout
        }

        rootLayout = RelativeLayout(context)
        rootLayout?.id = View.generateViewId()
        val lp = ViewGroup.LayoutParams(
            SETTING_WIDTH,
            ViewGroup.LayoutParams.MATCH_PARENT
        )

        rootLayout?.x = -1 * SETTING_WIDTH.toFloat()
        rootLayout?.layoutParams = lp
        rootLayout?.setBackgroundColor(0xFFEEE9E9.toInt())

        listener?.setSlideCallback(object: SlideCallback {
            override fun doHide() {
                animator?.reverse()
            }
        })
        initLayout()
        slideToDisplay(0F, SETTING_WIDTH.toFloat())
        return rootLayout
    }

    private fun initLayout() {
        val container = RelativeLayout(context)
        container.id = View.generateViewId()
        container.setBackgroundColor(Color.WHITE)

        val lp = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )

        rootLayout?.addView(container, lp)

        val avatarView = BoyiaImageView(context, 54.dp)
        avatarView.id = View.generateViewId()
        val avatarParam = RelativeLayout.LayoutParams(
                108.dp,
                108.dp
        )

        avatarParam.addRule(RelativeLayout.ALIGN_PARENT_TOP, rootLayout!!.id)
        avatarParam.addRule(RelativeLayout.CENTER_HORIZONTAL)
        avatarParam.topMargin = 108.dp

        container.addView(avatarView, avatarParam)

        val nameView = TextView(context)
        nameView.text = "Anonymous"
        nameView.setTextColor(Color.BLACK)

        val nameParam = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )
        nameParam.addRule(RelativeLayout.BELOW, avatarView.id)
        nameParam.addRule(RelativeLayout.CENTER_HORIZONTAL)
        nameParam.topMargin = 12.dp
        nameParam.bottomMargin = 12.dp
        container.addView(nameView, nameParam)

        val loginButton = buildButton("login", 32.dp, container.id)
        loginButton.setOnClickListener {
            module.showLogin()
        }

        val aboutButton = buildButton("about", 0.dp, loginButton.id)
        aboutButton.setOnClickListener {
            module.showAbout()
        }

        avatarView.load("https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500")
    }

    private fun slideToDisplay(start: Float, end: Float) {
        MainScheduler.mainScheduler().sendJob {
            animator = ValueAnimator.ofFloat(start, end).apply {
                duration = 300
                interpolator = LinearInterpolator()
                addListener(object: Animator.AnimatorListener {
                    override fun onAnimationStart(animation: Animator?) {
                        listener?.onStart(animator!!.animatedValue as Float)
                    }

                    override fun onAnimationEnd(animation: Animator?) {
                        val value = animator!!.animatedValue as Float
                        val show = value > 0F
                        listener?.onEnd(show)
                        if (!show) {
                            module.hide()
                        }
                    }

                    override fun onAnimationCancel(animation: Animator?) {
                    }

                    override fun onAnimationRepeat(animation: Animator?) {
                    }

                })
                addUpdateListener {
                    if (animator == null) {
                        return@addUpdateListener
                    }
                    val start = -1 * SETTING_WIDTH.toFloat()
                    val value = animator!!.animatedValue as Float
                    rootLayout!!.x = start + value
                    listener?.onSlide(value, value / SETTING_WIDTH.toFloat())
                    BoyiaLog.d(TAG, "rootLayout.x = ${rootLayout!!.x} and anim.v = ${animator!!.animatedValue}")
                }
            }

            animator?.start()

        }
    }

    private fun buildButton(text: String, marginTop: Int, sid: Int): Button {
        val button = Button(context)
        button.id = View.generateViewId()
        button.text = text
        button.setTextColor(Color.BLACK)
        button.setBackgroundColor(Color.WHITE)

        val loginParam = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )
        loginParam.addRule(RelativeLayout.BELOW, sid)
        loginParam.topMargin = marginTop
        loginParam.bottomMargin = 1.dp
        rootLayout?.addView(button, loginParam)

        return button
    }

    override fun canPop(): Boolean {
        return false
    }

    override fun hide() {
        if (animator?.isRunning == true) {
            BoyiaLog.d(TAG, "hide() isRunning true")
            return
        }

        BoyiaLog.d(TAG, "hide() isRunning false")
        animator?.reverse()
    }
}