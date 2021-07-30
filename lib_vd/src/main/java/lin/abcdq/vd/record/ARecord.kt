package lin.abcdq.vd.record

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.util.Log
import java.util.concurrent.Executors

internal class ARecord {

    companion object {
        private const val DEFAULT_SAMPLE_RATE = 44100
        private const val DEFAULT_CHANNEL_LAYOUT = AudioFormat.CHANNEL_IN_STEREO
        private const val DEFAULT_SAMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT
    }

    private val mThread = Executors.newSingleThreadExecutor()
    private var mAudioRecord: AudioRecord? = null
    private var mCall: AudioRecordCall? = null
    private var mInterrupt = false

    init {
        val minBufferSize = AudioRecord.getMinBufferSize(
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT
        )
        if (AudioRecord.ERROR_BAD_VALUE == minBufferSize) {
            Log.e("ARecord", "AudioRecord init error")
        }
        mAudioRecord = AudioRecord(
            MediaRecorder.AudioSource.MIC,
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT,
            minBufferSize
        )
    }

    fun setCall(call: AudioRecordCall) {
        mCall = call
    }

    fun onStart() {
        mThread.execute {
            mAudioRecord?.startRecording()
            val sampleBuffer = ByteArray(4096)
            try {
                while (!Thread.currentThread().isInterrupted && !mInterrupt) {
                    val result: Int = mAudioRecord?.read(sampleBuffer, 0, 4096) ?: 0
                    if (result > 0) {
                        mCall?.onCall(sampleBuffer)
                    }
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    fun onStop() {
        mInterrupt = true
        mAudioRecord?.release()
        mAudioRecord = null
    }

    interface AudioRecordCall {
        fun onCall(byteArray: ByteArray)
    }
}