package com.polytron.yolo11ncnncompose

import android.graphics.PixelFormat
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

@Composable
fun CameraScreen() {
    AndroidView(
        modifier = Modifier
            .fillMaxSize(),
        factory = { context ->
            SurfaceView(context).apply {
                holder.setFormat(PixelFormat.RGBA_8888)
                holder.addCallback(context as SurfaceHolder.Callback)
                (context as? MainActivity)?.surfaceView = this
            }
        }
    )
}
