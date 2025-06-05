package com.polytron.yolo11ncnncompose.di

import com.polytron.yolo11ncnncompose.socket.SocketViewModel
import org.koin.dsl.module

val appModule = module {
    single { SocketViewModel() }
}
