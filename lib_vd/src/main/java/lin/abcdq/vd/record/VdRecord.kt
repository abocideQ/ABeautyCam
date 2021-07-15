package lin.abcdq.vd.record

class VdRecord {

    private var recording = false

    private val mARecord = ARecord()

    fun onSource(
        outUrl: String,
        w: Int,
        h: Int,
        vBitRate: Long,
        fps: Int
    ) {
        native_vdRecord_onSource(outUrl, w, h, vBitRate, fps)
        mARecord.setCall(object : ARecord.AudioRecordCall {
            override fun onCall(byteArray: ByteArray) {
                onBufferAudio(byteArray)
            }
        })
    }

    fun onStart() {
        recording = true
        mARecord.onStart()
        native_vdRecord_onStart()
    }

    fun onStop() {
        recording = false
        mARecord.onStop()
        native_vdRecord_onStop()
    }

    fun onBufferVideo(format: Int, w: Int, h: Int, data: ByteArray) {
        native_vdRecord_onBufferVideo(format, w, h, data)
    }

    fun onGetBufferVideo(): ByteArray {
        return native_vdRecord_onGetBufferVideo()
    }

    fun onBufferAudio(data: ByteArray) {
        native_vdRecord_onBufferAudio(data)
    }

    init {
        System.loadLibrary("vd_make")
    }

    private external fun native_vdRecord_onSource(
        outUrl: String,
        w: Int,
        h: Int,
        vBitRate: Long,
        fps: Int
    )

    private external fun native_vdRecord_onStart()

    private external fun native_vdRecord_onStop()

    private external fun native_vdRecord_onBufferVideo(format: Int, w: Int, h: Int, data: ByteArray)

    private external fun native_vdRecord_onGetBufferVideo(): ByteArray

    private external fun native_vdRecord_onBufferAudio(data: ByteArray)

}