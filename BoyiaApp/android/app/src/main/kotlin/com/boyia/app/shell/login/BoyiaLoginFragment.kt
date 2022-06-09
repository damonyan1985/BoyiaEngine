package com.boyia.app.shell.login

import android.graphics.Color
import android.os.Bundle
import android.view.*
import android.widget.*
import com.boyia.app.shell.R
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.util.UnderlineEditText
import com.boyia.app.shell.util.dp

class BoyiaLoginFragment(private val module: LoginModule): NavigationFragment() {
    private var rootLayout: RelativeLayout? = null
    private var userInput: EditText? = null
    private var passwordInput: EditText? = null

    override fun createView(): View? {
        if (rootLayout != null) {
            val parant = rootLayout?.parent as ViewGroup
            parant.removeView(rootLayout)
            return rootLayout
        }

        initLayout()
        return rootLayout
    }

    private fun initLayout() {
        rootLayout = RelativeLayout(context)
        rootLayout?.setBackgroundColor(0xFF4F4F4F.toInt())

        val loginInputLayout = LinearLayout(context)
        loginInputLayout.orientation = LinearLayout.VERTICAL
        loginInputLayout.id = View.generateViewId()

        val userItem = initInputItem(R.drawable.person) { edit ->
            userInput = edit
        }
        val passwordItem = initInputItem(R.drawable.lock) { edit ->
            passwordInput = edit
        }

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

    private fun initInputItem(rid: Int, viewCallback: (edit: EditText) -> Unit): View {
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

        viewCallback(editText)
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
        forwardButton.setOnClickListener {
            module.login(userInput?.text.toString(), passwordInput?.text.toString())
        }
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
}