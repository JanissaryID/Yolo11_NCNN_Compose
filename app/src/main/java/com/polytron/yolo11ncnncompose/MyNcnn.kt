package com.polytron.yolo11ncnncompose

import android.content.res.AssetManager
import android.view.Surface

class MyNcnn {

    external fun loadModel(mgr: AssetManager): Boolean
    external fun openCamera(facing: Int): Boolean
    external fun closeCamera(): Boolean
    external fun setOutputWindow(surface: Surface): Boolean
    external fun getNumber(): Int

    companion object {
        init {
            System.loadLibrary("myncnn")
        }
    }
}