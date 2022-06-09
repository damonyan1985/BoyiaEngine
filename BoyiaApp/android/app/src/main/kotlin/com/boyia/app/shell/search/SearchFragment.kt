package com.boyia.app.shell.search

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.Text
import androidx.compose.material.TextField
import androidx.compose.material.TextFieldDefaults
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.ComposeView
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.util.dpx

class SearchFragment: NavigationFragment() {
    override fun createView(): View {
        return ComposeView(requireContext()).apply {
            setContent {
                buildLayout()
            }
        }
    }

    @Composable
    fun buildLayout() {
        Column(modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight()
                .padding(top = dpx(12))
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
                    shape = RoundedCornerShape(dpx(35)),
                    modifier = Modifier
                            .width(dpx(540))
                            .height(dpx(70)),
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