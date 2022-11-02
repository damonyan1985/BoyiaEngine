package com.boyia.app.shell.search

import android.view.View
import androidx.compose.foundation.*

import androidx.compose.runtime.*
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
import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.graphics.BlendMode
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.Path
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.loader.http.HTTPFactory
import com.boyia.app.loader.mue.MainScheduler
import com.boyia.app.shell.R
import com.boyia.app.shell.home.BoyiaDownloadMask
import com.boyia.app.shell.model.*
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.update.Downloader
import com.boyia.app.shell.util.CommonFeatures
import com.boyia.app.shell.util.dpx
import com.boyia.app.shell.util.toDp
import com.boyia.app.shell.util.CommonFeatures.marginTop
import com.boyia.app.shell.util.CommonFeatures.marginLeft
import com.boyia.app.shell.util.dpf

class SearchFragment(private val module: SearchModule): NavigationFragment() {
    //var appList = mutableStateListOf<BoyiaAppItem>()
    companion object {
        const val TAG = "SearchFragment"
    }
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
                    placeholder = { Text("Please input search key")},
                    maxLines = 1
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
                verticalAlignment = Alignment.CenterVertically,
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
            buildInstallButton()
//            TextButton(onClick = { },
//                    modifier = Modifier
//                            .align(alignment = Alignment.CenterVertically)
//                            .width(dpx(120))
//                            .height(dpx(72)),
//                    border = BorderStroke(dpx(2), Color.Green),
//                    shape = RoundedCornerShape(dpx(36)),
//            ) {
//                Text(text = "Install", color = Color.Green, fontWeight = FontWeight.Medium)
//            }

        }
    }

    @Composable
    fun buildInstallButton() {
        val completed = !Downloader().initDownloadData(BoyiaDownloadMask.TEST_URL)

        // by关键字代理行为，会生成一个对象
        // remember是一个函数，后面的{}也是一个函数，remember调用
        // currentComposer.cache来调用{}暂存和返回一个MutableState对象
        var updateProgress by remember {
            if (completed) mutableStateOf(1F) else mutableStateOf(0F)
        }

        TextButton(onClick = {
            BoyiaLog.d(TAG, "buildInstallButton updateProgress=$updateProgress");
            if (updateProgress > 0F) {
                return@TextButton
            }
            // 初始化时给个很小的值
            updateProgress = 0.001F

            Downloader(object : Downloader.DownLoadProgressListener {
                override fun onProgress(current: Long, size: Long) {
                    if (size == 0L) {
                        return
                    }

                    updateProgress = current.toFloat() / size.toFloat()
                }

                override fun onCompleted() {
                    updateProgress = 1F
                }
            }).download(BoyiaDownloadMask.TEST_URL)
        },
                modifier = Modifier
                        .width(dpx(120))
                        .height(dpx(72)),
                border = BorderStroke(dpx(2),
                        if (updateProgress == 1F) Color.Gray else Color.Green),
                shape = RoundedCornerShape(dpx(36)),
                contentPadding = PaddingValues(
                        start = 0.dp,
                        end = 0.dp,
                        top = 0.dp,
                        bottom = 0.dp)
        ) {
            Box(modifier = Modifier
                    .fillMaxWidth()
                    .fillMaxHeight(),
                    contentAlignment = Alignment.Center
            ) {
                Text(text = (if (updateProgress == 1F) "Open" else "Install"),
                        color = (if (updateProgress == 1F) Color.Gray else Color.Green), fontWeight = FontWeight.Medium)

                if (updateProgress != 1F) {
                    Canvas(modifier = Modifier
                            .fillMaxWidth()
                            .fillMaxHeight()
                    ) {

                        var path = Path().apply {
                            moveTo(0F, 0F)
                            lineTo(size.width * updateProgress, 0F)
                            lineTo(size.width * updateProgress, size.height)
                            lineTo(0F, size.height)
                            close()
                        }

                        drawPath(path = path, color = Color(0x3300FF00))
                    }
                }
            }

        }

    }
}