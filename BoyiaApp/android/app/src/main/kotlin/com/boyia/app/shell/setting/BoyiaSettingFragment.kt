package com.boyia.app.shell.setting

import android.animation.Animator
import android.animation.ValueAnimator
import android.annotation.SuppressLint
import android.app.Activity
import android.graphics.Color
import android.graphics.Paint
import android.os.Bundle
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.LinearInterpolator
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.image.BoyiaImageView
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.api.IPickImageLoader
import com.boyia.app.shell.model.*
import com.boyia.app.shell.module.BaseFragment
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideListener
import com.boyia.app.shell.setting.BoyiaSettingModule.SlideCallback
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.dp
import com.mikepenz.iconics.IconicsColor
import com.mikepenz.iconics.IconicsDrawable
import com.mikepenz.iconics.typeface.library.googlematerial.GoogleMaterial
import com.mikepenz.iconics.typeface.library.googlematerial.OutlinedGoogleMaterial
import com.mikepenz.iconics.utils.color
import com.mikepenz.iconics.utils.colorInt

class BoyiaSettingFragment(private val module: BoyiaSettingModule) : BaseFragment() {
    companion object {
        const val TAG = "BoyiaSettingFragment"
        const val DEFAULT_AVATAR = "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500"
        val SETTING_WIDTH = 320.dp
    }

    override fun customTag(): String = TAG

    private var rootLayout: RelativeLayout? = null
    private var animator: ValueAnimator? = null
    private var listener: SlideListener? = null
    private var avatarView: BoyiaImageView? = null
    private var nameView: TextView? = null
    private var loginButton: Button? = null
    private var avatarContainer: FrameLayout? = null
    private var cameraView: ImageView? = null

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
        rootLayout?.let {
            it.id = View.generateViewId()
            val lp = ViewGroup.LayoutParams(
                    SETTING_WIDTH,
                    CommonFeatures.getFragmentHeight(requireActivity())
            )
            it.x = -1 * SETTING_WIDTH.toFloat()
            it.layoutParams = lp
            it.setBackgroundColor(0xFFEEE9E9.toInt())
        }

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
        val info = BoyiaLoginInfo.instance()

        val container = RelativeLayout(context)
        container.id = View.generateViewId()
        container.setBackgroundColor(Color.WHITE)

        val lp = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )

        rootLayout?.addView(container, lp)

        initAvatar(info, container)

        nameView = TextView(context).apply {
            text = info.user?.nickname ?: "Anonymous"
            setTextColor(Color.BLACK)
        }

        val nameParam = RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        ).apply {
            addRule(RelativeLayout.BELOW, avatarContainer!!.id)
            addRule(RelativeLayout.CENTER_HORIZONTAL)
            topMargin = 12.dp
            bottomMargin = 12.dp
        }

        container.addView(nameView, nameParam)


        val loginText = if (info.token == null) "login" else "logout"
        loginButton = buildButton(loginText, 32.dp, container.id)
        loginButton?.setOnClickListener {
            if (info.token == null) {
                module.showLogin()
            } else {
                info.logout()
                setUserInfo(null)
            }
        }

        val aboutButton = buildButton("about", 0.dp, loginButton!!.id)
        aboutButton.setOnClickListener {
            module.showAbout()
        }
    }

    private fun initAvatar(info: BoyiaLoginInfo, container: RelativeLayout) {
        avatarContainer = FrameLayout(requireContext())
        avatarContainer?.id = View.generateViewId()

        avatarView = BoyiaImageView(context, 54.dp)
        avatarView?.id = View.generateViewId()
        avatarView?.setOnClickListener {
            if (info.isLogin()) {
                module.moduleContext()?.pickImage(object : IPickImageLoader {
                    override fun onImage(path: String) {
                        BoyiaModelUtil.request(
                                url = BoyiaModelUtil.UPLOAD_URL,
                                upload = true,
                                data = path,
                                headers = mapOf("User-Token" to (info.token ?: "none")),
                                cb = object : BoyiaModelUtil.ModelDataCallback<BoyiaUploadData> {
                                    override fun onLoadData(data: BoyiaUploadData) {
                                        info.user?.avatar = data.url
                                        info.flush()
                                        avatarView?.load(BoyiaModelUtil.getImageUrlWithToken(data.url))
                                        BoyiaLoginModel.update()
                                    }
                                }
                        )
                    }
                })
            }
        }

        avatarContainer?.addView(avatarView)

        val avatarParam = RelativeLayout.LayoutParams(
                108.dp,
                108.dp
        )

        avatarParam.addRule(RelativeLayout.ALIGN_PARENT_TOP, rootLayout!!.id)
        avatarParam.addRule(RelativeLayout.CENTER_HORIZONTAL)
        avatarParam.topMargin = 108.dp + BoyiaUtils.getStatusBarHeight(context as Activity)

        container.addView(avatarContainer, avatarParam)
        avatarView?.load(BoyiaModelUtil.getImageUrlWithToken(info.user?.avatar) ?: DEFAULT_AVATAR)

        if (info.isLogin()) {
            initCamera()
        }
    }

    private fun initCamera() {
        if (cameraView != null) {
            return
        }
        cameraView = ImageView(context)
        val drawable = IconicsDrawable(requireContext(), OutlinedGoogleMaterial.Icon.gmo_photo_camera)
        //val drawable = IconicsDrawable(requireContext(), GoogleMaterial.Icon.gmd_photo_camera)
                .apply {
                    sizeXPx = 36.dp
                    sizeYPx = 36.dp
                    colorInt = 0x99000000.toInt()
                }
        cameraView?.setImageDrawable(drawable)

        val cameraParam = FrameLayout.LayoutParams(
                30.dp,
                30.dp
        )

        cameraParam.gravity = Gravity.BOTTOM.or(Gravity.RIGHT)
        avatarContainer?.addView(cameraView, cameraParam)
    }

    private fun slideToDisplay(start: Float, end: Float) {
        MainScheduler.mainScheduler().sendJob {
            animator = ValueAnimator.ofFloat(start, end).apply {
                duration = 300
                interpolator = LinearInterpolator()
                addListener(object: Animator.AnimatorListener {
                    override fun onAnimationStart(animation: Animator) {
                        listener?.onStart(animator!!.animatedValue as Float)
                    }

                    override fun onAnimationEnd(animation: Animator) {
                        val value = animator!!.animatedValue as Float
                        val show = value > 0F
                        listener?.onEnd(show)
                        if (!show) {
                            module.hide()
                        }
                    }

                    override fun onAnimationCancel(animation: Animator) {
                    }

                    override fun onAnimationRepeat(animation: Animator) {
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

    @SuppressLint("SetTextI18n")
    fun setUserInfo(info: BoyiaUserInfo?) {
        avatarView?.load(BoyiaModelUtil.getImageUrlWithToken(info?.avatar) ?: DEFAULT_AVATAR)
        nameView?.text = info?.nickname ?: "Anonymous"

        loginButton?.text =  if (info == null) "login" else "logout"

        if (info != null) {
            initCamera()
        } else {
            avatarContainer?.removeView(cameraView)
            cameraView = null
        }
    }
}