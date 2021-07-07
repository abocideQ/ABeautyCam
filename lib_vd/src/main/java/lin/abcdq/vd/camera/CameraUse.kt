package lin.abcdq.vd.camera

import android.content.Context
import android.os.Build
import android.util.Size
import android.view.Surface
import android.view.TextureView
import androidx.annotation.RequiresApi
import lin.abcdq.vd.camera.wrap.CameraWrap
import lin.abcdq.vd.camera.wrap.CameraWrapCall

@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
class CameraUse(context: Context) {

    private val mCameraWrap = CameraWrap(context)
    private var mCameraSizes: Array<Size>? = null
    private var mSurface: Surface? = null

    fun open(surface: Surface) {
        mSurface = surface
        mCameraWrap.openCamera(mCameraWrap.facing, mSurface)
    }

    fun open() {
        mCameraWrap.openCamera(mCameraWrap.facing, null)
    }

    fun close() {
        mCameraSizes = null
        mSurface = null
        mCameraWrap.stopCamera()
    }

    fun setCall(cameraWrapCall: CameraWrapCall) {
        mCameraWrap.setCall(cameraWrapCall)
    }

    fun capture() {
        mCameraWrap.capture()
    }

    fun invalidate() {
        mCameraWrap.stopCamera()
        if (mSurface == null) mCameraWrap.openCamera(mCameraWrap.facing, null)
        else mCameraWrap.openCamera(mCameraWrap.facing, mSurface)
    }

    fun facing(): String {
        return mCameraWrap.facing
    }

    fun switch() {
        mCameraWrap.stopCamera()
        mCameraWrap.facing = when (mCameraWrap.facing) {
            CameraWrap.CAMERA_FRONT -> CameraWrap.CAMERA_BACK
            CameraWrap.CAMERA_BACK -> CameraWrap.CAMERA_FRONT
            else -> CameraWrap.CAMERA_EXTERNAL
        }
        if (mSurface == null) mCameraWrap.openCamera(mCameraWrap.facing, null)
        else mCameraWrap.openCamera(mCameraWrap.facing, mSurface)
    }

    fun resize(size: Size, textureView: TextureView) {
        mCameraWrap.stopCamera()
        mCameraWrap.updatePreviewSize(size)
        mCameraWrap.updateCaptureSize(size)
        textureView.surfaceTexture?.setDefaultBufferSize(size.width, size.height)
        mSurface = Surface(textureView.surfaceTexture ?: return)
        mCameraWrap.openCamera(mCameraWrap.facing, mSurface)
    }

    fun resize(size: Size) {
        mCameraWrap.stopCamera()
        mCameraWrap.updatePreviewSize(size)
        mCameraWrap.updateCaptureSize(size)
        mCameraWrap.openCamera(mCameraWrap.facing, null)
    }

    fun getPreviewSizes(): Array<Size>? {
        if (mCameraSizes == null) mCameraSizes = mCameraWrap.getPreviewSizes()
        return mCameraSizes
    }

    fun getPreviewSize(): Size? {
        return mCameraWrap.getPreviewSize()
    }
}