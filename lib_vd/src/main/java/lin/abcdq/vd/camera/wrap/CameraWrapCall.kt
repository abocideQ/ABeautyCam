package lin.abcdq.vd.camera.wrap

interface CameraWrapCall {
    fun onPreview(byteArray: ByteArray, width: Int, height: Int)
    fun onCapture(byteArray: ByteArray, width: Int, height: Int)
}