package com.boyia.app.shell.search

import android.view.View

import androidx.compose.runtime.*
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier

import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.http.HTTPFactory
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.R
import com.boyia.app.shell.model.*
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.dpx
import com.boyia.app.shell.util.toDp
import com.boyia.app.shell.util.CommonFeatures.marginTop
import com.boyia.app.shell.util.CommonFeatures.marginLeft

class SearchFragment(private val module: SearchModule): NavigationFragment() {
    //var appList = mutableStateListOf<BoyiaAppItem>()
    var model = BoyiaAppSearchModel()

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
            marginTop(top = 20)
            buildAppList()
        }
    }

    @Composable
    fun buildHeader() {
        var searchKey by remember { mutableStateOf("") }
        Row {
            Spacer(modifier = Modifier.width(dpx(20)))
            Image(
                    //painter = rememberImagePainter(data = "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500"),
                    painter = painterResource(id = R.drawable.login_back),
                    modifier = Modifier
                            .size(dpx(64))
                            .align(alignment = Alignment.CenterVertically)
                            .pointerInput(Unit) {
                                detectTapGestures(onTap = {
                                    hide()
                                })
                            },
                    contentScale = ContentScale.Fit,
                    contentDescription = "",
            )
            Spacer(modifier = Modifier.width(dpx(10)))
            TextField(searchKey,
                    { searchKey = it },
                    shape = CircleShape,
                    modifier = Modifier
                            .width(dpx(460)),
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
            Spacer(modifier = Modifier.width(dpx(10)))
            TextButton(onClick = { model.searchAppList(searchKey)},
                    modifier = Modifier
                            .align(alignment = Alignment.CenterVertically)
                            .background(Color.Gray, RoundedCornerShape(50)) // 要先设置背景再设置宽高，不然背景会超出宽高
                            .width(dpx(120))
                            .height(dpx(72)),
                    border = BorderStroke(dpx(4), Color(0xFFD3D3D3)),
                    shape = RoundedCornerShape(dpx(36)),
            ) {
                Text(text = "Search", color = Color.White, fontWeight = FontWeight.Medium)
            }
        }
    }

    @Composable
    fun buildAppList() {
        LazyColumn {
            items(model.appList.size) { index ->
                //model.appList[index].name?.let { Text(text = it) }
                buildSearchAppItem(model.appList[index])
            }
        }
    }

    @Composable
    fun buildSearchAppItem(item: BoyiaAppItem) {
        Row(modifier = Modifier
                .height(dpx(164))
                .fillMaxWidth()
                .padding(start = dpx(20), end = dpx(20))
                .clickable {
                    // TODO
                },
                horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Row(modifier = Modifier
                    .height(dpx(164))) {
                CommonFeatures.ComposeBoyiaImageView(
                        width = 164.dpx,
                        height = 164.dpx,
                        radius = 12.dpx,
                        url = item.cover
                )
                marginLeft(left = 20)
                Column(modifier = Modifier
                        .fillMaxHeight()
                        .padding(top = dpx(20), bottom = dpx(20)),
                        verticalArrangement = Arrangement.SpaceBetween,
                ) {
                    item.name?.let { Text(
                            it,
                            modifier = Modifier,
                            color = Color.Black,
                            fontSize = 18.sp,
                            fontStyle = FontStyle.Normal,
                            fontWeight = FontWeight.Bold,
                            ) }
                    Text(text = "AppID: ${item.appId}",
                            color = Color(0xFFD3D3D3),
                            fontSize = 12.sp,
                    )
                }
            }
            TextButton(onClick = { },
                    modifier = Modifier
                            .align(alignment = Alignment.CenterVertically)
                            .width(dpx(120))
                            .height(dpx(72)),
                    border = BorderStroke(dpx(2), Color.Green),
                    shape = RoundedCornerShape(dpx(36)),
            ) {
                Text(text = "Install", color = Color.Green, fontWeight = FontWeight.Medium)
            }
        }
    }
}