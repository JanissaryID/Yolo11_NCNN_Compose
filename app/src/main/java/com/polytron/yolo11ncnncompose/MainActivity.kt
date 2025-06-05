package com.polytron.yolo11ncnncompose

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.WindowManager
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.polytron.yolo11ncnncompose.socket.SocketViewModel
import com.polytron.yolo11ncnncompose.ui.theme.Yolo11NCNNComposeTheme
import org.koin.compose.koinInject

class MainActivity : ComponentActivity(), SurfaceHolder.Callback {

    private val REQUEST_CAMERA = 100
    private val myNcnn = MyNcnn()
    private var facing = 0
    lateinit var surfaceView: SurfaceView

    @OptIn(ExperimentalMaterial3Api::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        setContent {
            MaterialTheme {
//                val myViewModel : SocketViewModel = koinInject()
                val ip = "10.8.50.177"
//                myViewModel.connect(ip, myNcnn = myNcnn)
                myNcnn.connect(ip, 8888)

                Scaffold(
                    topBar = {
                        TopAppBar(
                            title = { Text(text = "NCNN Compose") },
                            actions = {
                                IconButton(
                                    onClick = {
                                    val newFacing = 1 - facing
                                    myNcnn.closeCamera()
                                    myNcnn.openCamera(newFacing)
                                    facing = newFacing
                                }) {
                                    Icon(
                                        imageVector = Icons.Default.Refresh,
                                        contentDescription = "Info"
                                    )
                                }
                                IconButton(
                                    onClick = {
                                        Log.i("Number", "Number: ${myNcnn.getNumber()}")
                                    }) {
                                    Icon(
                                        imageVector = Icons.Default.Info,
                                        contentDescription = "Get"
                                    )
                                }
                            }
                        )
                    }
                ) { paddingValues ->
                    Box(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(paddingValues),
                        contentAlignment = Alignment.Center
                    ) {
                        CameraScreen()
                    }
                }
            }
        }

        // Check permission on startup
        if (ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.CAMERA
            ) == PackageManager.PERMISSION_DENIED
        ) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.CAMERA),
                REQUEST_CAMERA
            )
        } else {
            myNcnn.openCamera(facing)
        }

        val retInit = myNcnn.loadModel(assets)
        if (!retInit) {
            Log.e("MainActivity", "yolo11ncnn loadModel failed")
        }
    }

    override fun onResume() {
        super.onResume()
        myNcnn.openCamera(facing)
    }

    override fun onPause() {
        super.onPause()
        myNcnn.closeCamera()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {}

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        myNcnn.setOutputWindow(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {}
}