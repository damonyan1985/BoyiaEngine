package com.boyia.app.shell.search

import android.view.View
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.Text
import androidx.compose.material.TextField
import androidx.compose.material.TextFieldDefaults
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.ComposeView
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.util.dpx
import com.boyia.app.shell.util.toDp

class SearchFragment: NavigationFragment() {
    override fun createView(): View {
        val view = ComposeView(requireContext()).apply {
            setContent {
                buildLayout()
            }
        }

        view.setBackgroundColor(0xFFFFFFFF.toInt())
        return view
    }

    @Composable
    fun buildLayout() {
        Column(modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight()
                .padding(top = dpx(12) + toDp(value = BoyiaUtils.getStatusBarHeight(activity)))
                .background(color = Color.White)) {
            buildHeader()
        }
    }

    @Composable
    fun buildHeader() {
        var searchKey by remember { mutableStateOf("") }
        Row {
            Spacer(modifier = Modifier.width(dpx(60)))
            TextField(searchKey,
                    { searchKey = it },
                    shape = CircleShape,
                    modifier = Modifier
                            .width(dpx(540)),
                            //.height(dpx(70)),
                    colors = TextFieldDefaults.textFieldColors(
                            //textColor = Color.Gray,
                            disabledTextColor = Color.Transparent,
                            //backgroundColor = Color.White,
                            focusedIndicatorColor = Color.Transparent,
                            unfocusedIndicatorColor = Color.Transparent,
                            disabledIndicatorColor = Color.Transparent
                    ),
                    placeholder = { Text("Please input search key")}
            )
        }
    }

    @Composable
    fun buildAppList() {
        LazyColumn {

        }
    }
}