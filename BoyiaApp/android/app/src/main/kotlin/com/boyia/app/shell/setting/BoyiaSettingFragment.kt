package com.boyia.app.shell.setting

import android.animation.Animator
import android.animation.ValueAnimator
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.LinearInterpolator
import android.widget.RelativeLayout
import androidx.fragment.app.Fragment
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils.dp
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideListener
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideCallback

class BoyiaSettingFragment(private val module: BoyiaSettingModule) : Fragment() {
    companion object {
        const val TAG = "BoyiaSettingFragment"
    }

    private val SETTING_WIDTH = dp(200)

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
        val lp = ViewGroup.LayoutParams(
            SETTING_WIDTH,
            ViewGroup.LayoutParams.MATCH_PARENT
        )

        rootLayout?.x = -1 * SETTING_WIDTH.toFloat()
        rootLayout?.layoutParams = lp
        rootLayout?.setBackgroundColor(0xFFEDEDED.toInt())

        listener?.setSlideCallback(object: SlideCallback {
            override fun doHide() {
                animator?.reverse()
            }
        })
        slideToDisplay(0F, SETTING_WIDTH.toFloat())
        return rootLayout
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
                        val show = value > 0F;
                        listener?.onEnd(show)
                        if (!show) {
                            module.dispose()
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
}