package com.polytron.yolo11ncnncompose

import android.content.res.AssetManager
import android.view.Surface

class YOLO11Ncnn {

    external fun loadModel(mgr: AssetManager): Boolean
    external fun openCamera(facing: Int): Boolean
    external fun closeCamera(): Boolean
    external fun setOutputWindow(surface: Surface): Boolean

    companion object {
        init {
            System.loadLibrary("yolo11ncnn")
        }
    }
}