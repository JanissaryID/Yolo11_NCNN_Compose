package com.polytron.yolo11ncnncompose.di

import android.app.Application
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin

class MyApp : Application() {
    override fun onCreate() {
        super.onCreate()

        // Inisialisasi Koin
        startKoin {
            androidContext(this@MyApp)
            modules(appModule)
        }
    }
}
