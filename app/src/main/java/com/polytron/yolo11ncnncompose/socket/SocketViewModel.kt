package com.polytron.yolo11ncnncompose.socket

import android.util.Log
import com.polytron.yolo11ncnncompose.MyNcnn
import dev.icerock.moko.mvvm.livedata.LiveData
import dev.icerock.moko.mvvm.livedata.MutableLiveData
import dev.icerock.moko.mvvm.livedata.postValue
import dev.icerock.moko.mvvm.viewmodel.ViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.InputStream
import java.io.OutputStream
import java.net.ServerSocket
import java.net.Socket

class SocketViewModel : ViewModel() {

    private var socket: Socket? = null
    private var outputStream: OutputStream? = null

    private var _number: MutableLiveData<Int> = MutableLiveData(-1)
    val number: LiveData<Int> get() = _number

    fun setNumber(number: Int) {
        _number.value = number
    }

    fun connect(host: String, port: Int = 8888, myNcnn: MyNcnn) {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                socket = Socket(host, port)
                outputStream = socket?.getOutputStream()
                Log.d("SocketViewModel", "Connected to $host:$port")

                _number.postValue(myNcnn.getNumber())
            } catch (e: Exception) {
                Log.e("SocketViewModel", "Connection error: ${e.message}", e)
            }
        }
    }

    fun sendMessage(message: String) {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                if (outputStream == null) {
                    Log.e("SocketViewModel", "Not connected. Call connect() first.")
                    return@launch
                }
                outputStream?.write(message.toByteArray())
                outputStream?.flush()
                Log.d("SocketViewModel", "Sent message: $message")
            } catch (e: Exception) {
                Log.e("SocketViewModel", "Send error: ${e.message}", e)
            }
        }
    }

    fun disconnect() {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                outputStream?.close()
                socket?.close()
                outputStream = null
                socket = null
                Log.d("SocketViewModel", "Disconnected")
            } catch (e: Exception) {
                Log.e("SocketViewModel", "Disconnect error: ${e.message}", e)
            }
        }
    }

    override fun onCleared() {
        super.onCleared()
        // Pastikan socket ditutup saat ViewModel dihapus
        disconnect()
    }
}