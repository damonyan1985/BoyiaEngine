package com.boyia.app.shell.setting

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material.LinearProgressIndicator
import androidx.compose.material.Text
import androidx.compose.material.TextButton
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import coil.compose.rememberImagePainter
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.shell.R
import com.boyia.app.shell.home.BoyiaDownloadMask
import com.boyia.app.shell.module.NavigationFragment
import com.boyia.app.shell.update.Downloader
import com.boyia.app.shell.update.Downloader.DownLoadProgressListener
import com.boyia.app.shell.util.dpx
import com.boyia.app.shell.util.toDp

/**
 * 使用compose来编写关于页面
 */
class BoyiaAboutFragment: NavigationFragment() {
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
        Box(modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight()
                .background(color =  Color(0x08000000))
                .padding(top = toDp(BoyiaUtils.getStatusBarHeight(activity)))
        ) {
            Column(
                    modifier = Modifier
                            .fillMaxWidth()
                            .fillMaxHeight()
            ) {

                Column(
                        modifier = Modifier
                                .height(220.dpx)
                                .fillMaxWidth()
                        //.background(Color.White)
                ) {
                    Spacer(modifier = Modifier.height(60.dpx))
                    Image(
                            //painter = rememberImagePainter(data = "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500"),
                            painter = painterResource(id = R.drawable.ic_launcher),
                            modifier = Modifier
                                    .size(80.dpx)
                                    .align(alignment = Alignment.CenterHorizontally),
                            contentScale = ContentScale.Fit,
                            contentDescription = "",
                    )
                }
                //Spacer(modifier = Modifier.height(100.dp))
                versionButton()

                Spacer(modifier = Modifier.height(1.dpx))

                TextButton(
                        onClick = { /*TODO*/ },
                        modifier = Modifier
                                .background(Color.White)
                                .fillMaxWidth()
                                .height(60.dpx),
                        contentPadding = PaddingValues()

                ) {
                    Text(text = "Feature introduction", color = Color.Black)
                }

                Spacer(modifier = Modifier.height(1.dpx))

                TextButton(
                        onClick = { /*TODO*/ },
                        modifier = Modifier
                                .background(Color.White)
                                .fillMaxWidth()
                                .height(60.dpx),
                        contentPadding = PaddingValues()

                ) {
                    Text(text = "Register", color = Color.Black)
                }

                Spacer(modifier = Modifier.height(1.dpx))

                TextButton(
                        onClick = { /*TODO*/ },
                        modifier = Modifier
                                .background(Color.White)
                                .fillMaxWidth()
                                .height(60.dpx),
                        contentPadding = PaddingValues()

                ) {
                    Text(text = "Feedback", color = Color.Black)
                }
            }
        }
    }

    @Composable
    fun versionButton() {
        val completed = !Downloader().initDownloadData(BoyiaDownloadMask.TEST_URL)

        var updateProgress by remember {
            if (completed) mutableStateOf(1F) else mutableStateOf(0F)
        }
        Row(modifier = Modifier
                .background(Color.White)
                .fillMaxWidth()
                .height(60.dpx)
                .clickable {
                    if (updateProgress > 0F) {
                        return@clickable
                    }
                    // 初始化时给个很小的值
                    updateProgress = 0.001F

                    Downloader(object : DownLoadProgressListener {
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
                horizontalArrangement = Arrangement.Center,
                verticalAlignment = Alignment.CenterVertically
        ) {
            when (updateProgress) {
                0F -> {
                    Text(text = "Version update", color = Color.Black, fontWeight = FontWeight.Medium)
                }
                1F -> {
                    Text(text = "Latest version", color = Color.Black, fontWeight = FontWeight.Medium)
                }
                else -> {
                    Box(contentAlignment = Alignment.Center) {
                        LinearProgressIndicator(
                                progress = updateProgress,
                                modifier = Modifier
                                        .fillMaxWidth()
                                        .height(60.dpx),
                                color = Color(0xFFD3D3D3),
                                backgroundColor = Color.White
                        )
                        Text(text = "Downloading...", color = Color.Black, fontWeight = FontWeight.Medium)
                    }
                }
            }
        }
    }
}