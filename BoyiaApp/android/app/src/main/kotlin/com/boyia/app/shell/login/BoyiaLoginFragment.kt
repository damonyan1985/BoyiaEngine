package com.boyia.app.shell.login

import android.animation.Animator
import android.animation.ValueAnimator
import android.graphics.Color
import android.os.Bundle
import android.view.*
import android.view.animation.Animation
import android.view.animation.AnimationUtils
import android.view.animation.LinearInterpolator
import android.widget.*
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentTransaction
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.R
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.setting.BoyiaSettingFragment
import com.boyia.app.shell.util.UnderlineEditText
import com.boyia.app.shell.util.dp

class BoyiaLoginFragment(private val module: LoginModule): BaseFragment() {
    private var rootLayout: RelativeLayout? = null
    private var animator: ValueAnimator? = null

    private val LOGIN_WIDTH = BoyiaUtils.getScreenSize().x.toFloat()

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout
        }

        initLayout()
        //rootLayout?.x = -LOGIN_WIDTH
//        rootLayout?.setOnKeyListener(object: View.OnKeyListener {
//            override fun onKey(v: View?, keyCode: Int, event: KeyEvent?): Boolean {
//                if (keyCode == KeyEvent.KEYCODE_BACK) {
//                    animator?.reverse()
//                    return true
//                }
//                return false
//            }
//        })

        //slideToDisplay(0F, LOGIN_WIDTH)
        return rootLayout
    }

    private fun initLayout() {
        rootLayout = RelativeLayout(context)
        rootLayout?.setBackgroundColor(0xFF4F4F4F.toInt())

        val loginInputLayout = LinearLayout(context)
        loginInputLayout.orientation = LinearLayout.VERTICAL
        loginInputLayout.id = View.generateViewId()

        val userItem = initInputItem(R.drawable.person)
        val passwordItem = initInputItem(R.drawable.lock)

        loginInputLayout.addView(userItem)
        loginInputLayout.addView(passwordItem)

        val loginInputParam = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
        )

        loginInputParam.addRule(RelativeLayout.CENTER_HORIZONTAL)
        loginInputParam.topMargin = 540.dp

        rootLayout?.addView(loginInputLayout, loginInputParam)

        initLoginButton(loginInputLayout.id)
    }

    private fun initInputItem(rid: Int): View {
        val inputLayout = LinearLayout(context)
        inputLayout.orientation = LinearLayout.HORIZONTAL

        val imageView = ImageView(context)
        imageView.setImageResource(rid)
        imageView.setColorFilter(0xFFCAE1FF.toInt())
        val imageViewParam = LinearLayout.LayoutParams(
                36.dp,
                36.dp
        )
        imageViewParam.gravity = Gravity.CENTER_VERTICAL
        inputLayout.addView(imageView, imageViewParam)

        val editText = UnderlineEditText(requireContext())
        editText.setBackgroundColor(Color.TRANSPARENT)
        val editTextParam = LinearLayout.LayoutParams(
                400.dp,
                120.dp
        )
        editTextParam.gravity = Gravity.CENTER_VERTICAL
        inputLayout.addView(editText, editTextParam)
        return inputLayout;
    }

    private fun initLoginButton(id: Int) {
        val backButton = ImageView(context)
        backButton.setImageResource(R.drawable.login_back)
        backButton.setColorFilter(0xFFCAE1FF.toInt())
        backButton.setOnClickListener {
            //animator?.reverse()
            module.hide()
        }
        val backButtonParam = LinearLayout.LayoutParams(
                64.dp,
                64.dp
        )

        val forwardButton = ImageView(context)
        forwardButton.setImageResource(R.drawable.login_forward)
        forwardButton.setColorFilter(0xFFCAE1FF.toInt())
        val forwardButtonParam = LinearLayout.LayoutParams(
                64.dp,
                64.dp
        )

        forwardButtonParam.leftMargin = 54.dp

        val buttonLayout = LinearLayout(context)
        buttonLayout.orientation = LinearLayout.HORIZONTAL
        buttonLayout.addView(backButton, backButtonParam)
        buttonLayout.addView(forwardButton, forwardButtonParam)

        val buttonLayoutParam = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )

        buttonLayoutParam.addRule(RelativeLayout.BELOW, id)
        buttonLayoutParam.addRule(RelativeLayout.CENTER_HORIZONTAL)
        buttonLayoutParam.topMargin = 120.dp

        rootLayout?.addView(buttonLayout, buttonLayoutParam)
    }

    private fun slideToDisplay(start: Float, end: Float) {
        MainScheduler.mainScheduler().sendJob {
            animator = ValueAnimator.ofFloat(start, end).apply {
                duration = 300
                interpolator = LinearInterpolator()
                addListener(object: Animator.AnimatorListener {
                    override fun onAnimationStart(animation: Animator?) {
                    }

                    override fun onAnimationEnd(animation: Animator?) {
                        val value = animator!!.animatedValue as Float
                        val show = value > 0F
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
                    val start = -1 * LOGIN_WIDTH
                    val value = animator!!.animatedValue as Float
                    rootLayout!!.x = start + value
                }
            }

            animator?.start()

        }
    }

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
        module.hide()
    }
}