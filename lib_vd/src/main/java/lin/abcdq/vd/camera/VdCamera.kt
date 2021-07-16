package lin.abcdq.vd.camera

import android.content.Context
import android.graphics.ImageFormat
import android.opengl.GLSurfaceView
import android.os.Build
import android.util.Log
import android.util.Size
import androidx.annotation.RequiresApi
import lin.abcdq.vd.camera.util.CAO
import lin.abcdq.vd.camera.wrap.CameraWrap
import lin.abcdq.vd.camera.wrap.CameraWrapCall
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
class VdCamera(context: Context, format: Int) : GLSurfaceView.Renderer {

    private var mFormat = format
    private var mCameraUse: CameraUse? = null

    private var mFaceModel = ""
    private var mEyesModel = ""
    private var mNoseModel = ""
    private var mMouthModel = ""
    private var mFace = false

    fun setSurface(surface: GLSurfaceView) {
        surface.setEGLContextClientVersion(3)
        surface.setRenderer(this)
        surface.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
    }

    fun onSwitch() {
        mCameraUse?.switch()
        if (mCameraUse?.facing() == CameraWrap.CAMERA_BACK) {
            native_vdCameraRender_onRotate(-90.0f, true)
        } else {
            native_vdCameraRender_onRotate(90.0f, false)
        }
    }

    fun onBuffer(): ByteArray? {
        return native_vdCameraRender_onBufferCapture()
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
        if (mFace) {
            native_vdCameraRender_onFaceInit(mFaceModel, mEyesModel, mNoseModel, mMouthModel)
        }
        mCameraUse?.setCall(object : CameraWrapCall {
            override fun onPreview(byteArray: ByteArray, width: Int, height: Int) {
                if (mFace) {
                    native_vdCameraRender_onFaceBuffer(mFormat, width, height, byteArray)
                } else {
                    native_vdCameraRender_onBuffer(mFormat, width, height, byteArray)
                }
            }

            override fun onCapture(byteArray: ByteArray, width: Int, height: Int) {
            }
        })
        if (mCameraUse?.facing() == CameraWrap.CAMERA_BACK) {
            native_vdCameraRender_onRotate(-90.0f, true)
        } else {
            native_vdCameraRender_onRotate(90.0f, false)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        native_vdCameraRender_onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        native_vdCameraRender_onDrawFrame()
    }

    init {
        copyModel2Local(context)
        mCameraUse = CameraUse(context)
        System.loadLibrary("vd_make")
    }

    private external fun native_vdCameraRender_onFaceInit(
        face: String,
        eyes: String,
        nose: String,
        mouth: String
    )

    private external fun native_vdCameraRender_onFaceBuffer(
        ft: Int,
        w: Int,
        h: Int,
        bytes: ByteArray
    )

    private external fun native_vdCameraRender_onBuffer(ft: Int, w: Int, h: Int, bytes: ByteArray)

    private external fun native_vdCameraRender_onBufferCapture(): ByteArray?

    private external fun native_vdCameraRender_onRotate(rotate: Float, modelRot: Boolean)

    private external fun native_vdCameraRender_onRelease()

    private external fun native_vdCameraRender_onSurfaceCreated()

    private external fun native_vdCameraRender_onSurfaceChanged(w: Int, h: Int)

    private external fun native_vdCameraRender_onDrawFrame()

    private fun copyModel2Local(context: Context) {
        mFace = true
        CAO.copyAssetsDirToSDCard(context, "opencv", context.obbDir.absolutePath)
        mFaceModel = context.obbDir.absolutePath + "/opencv/haarcascade_frontalface_default.xml"
        mEyesModel = context.obbDir.absolutePath + "/opencv/haarcascade_eye.xml"
        mNoseModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_nose.xml"
        mMouthModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_mouth.xml"
    }
}