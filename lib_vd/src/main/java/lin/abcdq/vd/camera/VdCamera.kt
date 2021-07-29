package lin.abcdq.vd.camera

import android.content.Context
import android.graphics.ImageFormat
import android.graphics.PixelFormat
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
class VdCamera(context: Context) : GLSurfaceView.Renderer {

    //图像采样格式：1.YUV420 (5~8ms)  2.NV21/12 (1~3ms)
    private var mFormat = 2
    private var mCameraUse: CameraUse? = null

    //人脸检测方式 :1.opencv(slow) 2.faceCNN(fucking slow) 3.NCNN(normal) 4.opencvTrack(fast but wheres my face)
    private var mFacePosition = 1

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
        mCameraUse?.setCall(object : CameraWrapCall {
            override fun onPreview(byteArray: ByteArray, width: Int, height: Int) {
                native_vdCameraRender_onBuffer(mFormat, width, height, byteArray)
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
        mCameraUse = CameraUse(context)
        System.loadLibrary("vd_make")
        when (mFacePosition) {
            1 -> onFaceCV(context)
            2 -> onFaceCnn(context)
            3 -> onFaceNCNN(context)
            4 -> onFaceCVTrack(context)
        }
    }

    private external fun native_vdCameraRender_onFace(
        s1: String,
        s2: String,
        s3: String,
        s4: String,
        s5: String,
        faceI: Int
    )

    private external fun native_vdCameraRender_onBuffer(ft: Int, w: Int, h: Int, bytes: ByteArray)

    private external fun native_vdCameraRender_onBufferCapture(): ByteArray?

    private external fun native_vdCameraRender_onRotate(rotate: Float, modelRot: Boolean)

    private external fun native_vdCameraRender_onRelease()

    private external fun native_vdCameraRender_onSurfaceCreated()

    private external fun native_vdCameraRender_onSurfaceChanged(w: Int, h: Int)

    private external fun native_vdCameraRender_onDrawFrame()

    private fun onFaceCV(context: Context) {
        CAO.copyAssetsDirToSDCard(context, "opencv", context.obbDir.absolutePath)
        val mFaceModel = context.obbDir.absolutePath + "/opencv/haarcascade_frontalface_alt.xml"
        val mEyesModel = context.obbDir.absolutePath + "/opencv/haarcascade_eye.xml"
        val mNoseModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_nose.xml"
        val mMouthModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_mouth.xml"
        native_vdCameraRender_onFace(
            mFaceModel,
            mEyesModel,
            mNoseModel,
            mMouthModel,
            "mAlignmentModel",
            1
        )
    }

    private fun onFaceCnn(context: Context) {
        CAO.copyAssetsDirToSDCard(context, "image", context.obbDir.absolutePath)
        native_vdCameraRender_onFace(
            "mFaceModel",
            "mEyesModel",
            "mNoseModel",
            "mMouthModel",
            "mAlignmentModel",
            2
        )
    }

    private fun onFaceNCNN(context: Context) {
        CAO.copyAssetsDirToSDCard(context, "ncnn", context.obbDir.absolutePath)
        CAO.copyAssetsDirToSDCard(context, "alignment", context.obbDir.absolutePath)
        val mAlignmentModel = context.obbDir.absolutePath + "/alignment/seeta_fa_v1.1.bin"
        native_vdCameraRender_onFace(
            context.obbDir.absolutePath + "/ncnn/",
            " mEyesModel ",
            " mNoseModel ",
            " mMouthModel ",
            mAlignmentModel,
            3
        )
    }

    private fun onFaceCVTrack(context: Context) {
        CAO.copyAssetsDirToSDCard(context, "opencv", context.obbDir.absolutePath)
        CAO.copyAssetsDirToSDCard(context, "alignment", context.obbDir.absolutePath)
        val mFaceModel = context.obbDir.absolutePath + "/opencv/haarcascade_frontalface_alt.xml"
        val mEyesModel = context.obbDir.absolutePath + "/opencv/haarcascade_eye.xml"
        val mNoseModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_nose.xml"
        val mMouthModel = context.obbDir.absolutePath + "/opencv/haarcascade_mcs_mouth.xml"
        val mAlignmentModel = context.obbDir.absolutePath + "/alignment/seeta_fa_v1.1.bin"
        native_vdCameraRender_onFace(
            mFaceModel,
            mEyesModel,
            mNoseModel,
            mMouthModel,
            mAlignmentModel,
            4
        )
    }
}