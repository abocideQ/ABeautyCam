package lin.abcdq.vd.camera

import android.content.Context
import android.graphics.ImageFormat
import android.opengl.GLSurfaceView
import android.os.Build
import android.util.Log
import android.util.Size
import androidx.annotation.RequiresApi
import lin.abcdq.vd.camera.wrap.CameraWrap
import lin.abcdq.vd.camera.wrap.CameraWrapCall
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
class VdCamera(context: Context, format: Int) : GLSurfaceView.Renderer {

    private var mFormat = format
    private var mCameraUse: CameraUse? = null

    fun setSurface(surface: GLSurfaceView) {
        surface.setEGLContextClientVersion(3)
        surface.setRenderer(this)
    }

    fun onSwitch() {
        mCameraUse?.switch()
        if (mCameraUse?.facing() == CameraWrap.CAMERA_BACK) {
            native_vdCameraRender_onRotate(90.0f)
        } else {
            native_vdCameraRender_onRotate(270.0f)
        }
    }

    fun onBuffer(): ByteArray {
        return native_vdCameraRender_onBuffer()
    }

    fun onOpen() {
        mCameraUse?.open()
    }

    fun onStop() {
        mCameraUse?.stop()
    }

    fun onRelease() {
        mCameraUse?.close()
        native_vdCameraRender_onRelease()
    }

    fun onSize(size: Size) {
        mCameraUse?.resize(size)
    }

    fun onSize(): Size? {
        return mCameraUse?.getPreviewSize()
    }

    fun onSizes(): Array<Size>? {
        return mCameraUse?.getPreviewSizes()
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        native_vdCameraRender_onSurfaceCreated()
        when (mFormat) {
            1 -> {
                CameraWrap.IMAGE_FORMAT = ImageFormat.YUV_420_888
            }
            2 -> {
                CameraWrap.IMAGE_FORMAT = ImageFormat.NV21
            }
            else -> {
                CameraWrap.IMAGE_FORMAT = ImageFormat.JPEG
            }
        }
        mCameraUse?.open()
        mCameraUse?.setCall(object : CameraWrapCall {
            override fun onPreview(byteArray: ByteArray, width: Int, height: Int) {
                native_vdCameraRender_onBuffer(mFormat, width, height, byteArray)
            }

            override fun onCapture(byteArray: ByteArray, width: Int, height: Int) {
            }
        })
        if (mCameraUse?.facing() == CameraWrap.CAMERA_BACK) {
            native_vdCameraRender_onRotate(90.0f)
        } else {
            native_vdCameraRender_onRotate(270.0f)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        native_vdCameraRender_onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        native_vdCameraRender_onDrawFrame()
    }

    init {
        mCameraUse = CameraUse(context)
        System.loadLibrary("vd_make")
    }

    private external fun native_vdCameraRender_onBuffer(ft: Int, w: Int, h: Int, bytes: ByteArray)
    private external fun native_vdCameraRender_onBuffer(): ByteArray
    private external fun native_vdCameraRender_onRotate(rotate: Float)
    private external fun native_vdCameraRender_onRelease()
    private external fun native_vdCameraRender_onSurfaceCreated()
    private external fun native_vdCameraRender_onSurfaceChanged(w: Int, h: Int)
    private external fun native_vdCameraRender_onDrawFrame()

}