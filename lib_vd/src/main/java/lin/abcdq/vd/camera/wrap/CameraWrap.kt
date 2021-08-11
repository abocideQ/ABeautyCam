package lin.abcdq.vd.camera.wrap

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.ImageFormat
import android.graphics.PixelFormat
import android.hardware.camera2.*
import android.hardware.camera2.CameraCaptureSession.CaptureCallback
import android.hardware.camera2.params.OutputConfiguration
import android.hardware.camera2.params.SessionConfiguration
import android.media.Image
import android.media.ImageReader
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.util.Range
import android.util.Size
import android.view.Surface
import androidx.annotation.RequiresApi
import lin.abcdq.ffmpeg.util.ImageUtils
import java.util.concurrent.Executor
import kotlin.math.abs

@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
internal class CameraWrap(context: Context) {

    companion object CameraFacing {

        var CAMERA_FRONT = "0"
        var CAMERA_BACK = "1"
        var CAMERA_EXTERNAL = "2"

        //ImageFormat.YUV_420_888 : 5~8 ms
        //ImageFormat.NV21 : 1~3 ms
        var IMAGE_FORMAT = ImageFormat.NV21

        //ImageFormat.YUV_420_888
        private const val IMAGE_READER_FORMAT = ImageFormat.YUV_420_888
    }

    var facing = CAMERA_FRONT

    fun getPreviewSizes(): Array<Size>? {
        val characteristics = mCameras[facing] ?: return null
        val configs = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
            ?: return null
        return configs.getOutputSizes(IMAGE_READER_FORMAT)
    }

    fun getPreviewSize(): Size? {
        return mPreviewSize
    }

    fun updatePreviewSize(size: Size) {
        mPreviewSize = size
    }

    fun updateCaptureSize(size: Size) {
        mCaptureSize = size
    }

    fun setCall(cameraWrapCall: CameraWrapCall) {
        mCall = cameraWrapCall
    }

    @SuppressLint("MissingPermission")
    fun openCamera(facing: String, surface: Surface?) {
        val characteristics = mCameras[facing] ?: return
        val fps = characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES)
        if (fps != null) {
            mFps = fps[0]
            for (f in fps) {
                val low = mFps?.lower ?: 0
                val up = mFps?.upper ?: 0
                if (f.lower <= low && f.upper >= up) mFps = f
            }
        }
        initHandler()
        mPreviewReader = ImageReader.newInstance(
            mPreviewSize?.width ?: 1, mPreviewSize?.height ?: 1, IMAGE_READER_FORMAT, 2
        )
        mPreviewReader?.setOnImageAvailableListener(
            {
                val image: Image? = it.acquireNextImage()
//                val image: Image? = it.acquireLatestImage()
                if (image != null) {
//                    val startTime = System.currentTimeMillis() // 获取开始时间
                    if (IMAGE_READER_FORMAT == ImageFormat.YUV_420_888) {
                        when (IMAGE_FORMAT) {
                            ImageFormat.YUV_420_888 -> {
                                mCall?.onPreview(
                                    ImageUtils.image2YUV420888(image),
                                    image.width,
                                    image.height
                                )
                            }
                            ImageFormat.NV21 -> {
                                mCall?.onPreview(
                                    ImageUtils.image2NV21(image),
                                    image.width,
                                    image.height
                                )
                            }
                            ImageFormat.JPEG -> {
                                mCall?.onPreview(
                                    ImageUtils.image2JPEG(image),
                                    image.width,
                                    image.height
                                )
                            }
                        }
//                        val endTime = System.currentTimeMillis() // 获取结束时间
//                        Log.e("camera acc ", "时间： " + (endTime - startTime) + "ms")
                    }
                    image.close()
                }
            },
            mHandler
        )
        mCaptureReader = ImageReader.newInstance(
            mPreviewSize?.width ?: 1, mPreviewSize?.height ?: 1, IMAGE_READER_FORMAT, 2
        )
        mCaptureReader?.setOnImageAvailableListener(
            {
                val image: Image? = it.acquireLatestImage()
                if (image != null) {
                    if (IMAGE_READER_FORMAT == ImageFormat.YUV_420_888) {
                        when (IMAGE_FORMAT) {
                            ImageFormat.YUV_420_888 -> {
                                mCall?.onCapture(
                                    ImageUtils.image2YUV420888(image),
                                    image.width,
                                    image.height
                                )
                            }
                            ImageFormat.NV21 -> {
                                mCall?.onCapture(
                                    ImageUtils.image2NV21(image),
                                    image.width,
                                    image.height
                                )
                            }
                            ImageFormat.JPEG -> {
                                mCall?.onCapture(
                                    ImageUtils.image2JPEG(image),
                                    image.width,
                                    image.height
                                )
                            }
                        }
                    }
                    image.close()
                }
            },
            mHandler
        )
        mCameraManager.openCamera(facing, object : CameraDevice.StateCallback() {
            override fun onOpened(camera: CameraDevice) {
                mCameraDevice = camera
                createPipe(surface)
            }

            override fun onDisconnected(camera: CameraDevice) {
                camera.close()
                mCameraDevice = null
            }

            override fun onError(camera: CameraDevice, error: Int) {
                camera.close()
                mCameraDevice = null
            }
        }, mHandler)
    }

    fun capture() {
        if (mCameraDevice == null) return
        val captureBuilder =
            mCameraDevice?.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE) ?: return
        captureBuilder.addTarget(mCaptureReader?.surface ?: return)
        captureBuilder.set(
            CaptureRequest.CONTROL_AF_MODE,
            CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE
        )
        val captureCallback: CaptureCallback = object : CaptureCallback() {
            override fun onCaptureCompleted(
                session: CameraCaptureSession,
                request: CaptureRequest,
                result: TotalCaptureResult
            ) {
                mCameraSession?.setRepeatingRequest(
                    mCameraRequest ?: return,
                    null,
                    mHandler ?: return
                )
            }
        }
        mCameraSession?.stopRepeating()
        mCameraSession?.abortCaptures()
        mCameraSession?.capture(captureBuilder.build(), captureCallback, mHandler)
    }

    fun stopCamera() {
        mCameraRequest = null
        mCameraSession?.close()
        mCameraSession = null
        mCameraDevice?.close()
        mCameraDevice = null
        mPreviewReader?.setOnImageAvailableListener(null, null)
        mPreviewReader?.close()
        mCaptureReader = null
        mCaptureReader?.setOnImageAvailableListener(null, null)
        mCaptureReader?.close()
        mCaptureReader = null
        mHandlerThread?.quitSafely()
        mHandlerThread = null
        mHandler?.removeCallbacksAndMessages(null)
        mHandler = null
    }

    private fun createPipe(surface: Surface?) {
        val captureBuilder =
            mCameraDevice?.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW) ?: return
        captureBuilder.addTarget(surface ?: mPreviewReader?.surface ?: return)
        captureBuilder.set(
            CaptureRequest.CONTROL_AE_MODE,
            CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE
        )
        captureBuilder.set(
            CaptureRequest.CONTROL_AE_MODE,
            CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH
        )
        if (mFps != null) {
            captureBuilder.set(
                CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE,
                mFps
            )
        }
        mCameraRequest = captureBuilder.build()
        val sessionCall = object : CameraCaptureSession.StateCallback() {
            override fun onConfigured(session: CameraCaptureSession) {
                mCameraSession = session
                mCameraSession?.setRepeatingRequest(
                    mCameraRequest ?: return,
                    null,
                    mHandler ?: return
                )
            }

            override fun onConfigureFailed(session: CameraCaptureSession) {
                session.close()
                mCameraSession = null
            }
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            val sessionConfiguration = SessionConfiguration(
                SessionConfiguration.SESSION_REGULAR,
                listOf(
                    OutputConfiguration(surface ?: mPreviewReader?.surface ?: return),
                    OutputConfiguration(mCaptureReader?.surface ?: return)
                ),
                Executor { command -> mHandler?.post(command ?: return@Executor) },
                sessionCall
            )
            mCameraDevice?.createCaptureSession(sessionConfiguration)
        } else {
            mCameraDevice?.createCaptureSession(
                listOf(surface ?: mPreviewReader?.surface, mCaptureReader?.surface),
                sessionCall,
                mHandler
            )
        }
    }

    private var mHandlerThread: HandlerThread? = null
    private var mHandler: Handler? = null
    private val mCameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
    private var mCameras = HashMap<String, CameraCharacteristics>()
    private var mCameraDevice: CameraDevice? = null
    private var mCameraRequest: CaptureRequest? = null
    private var mCameraSession: CameraCaptureSession? = null
    private val mDefaultPreviewSize = Size(1280, 720)
    private val mDefaultCaptureSize = Size(1280, 720)
    private var mFps: Range<Int>? = null
    private var mPreviewSize: Size? = null
    private var mCaptureSize: Size? = null

    private var mPreviewReader: ImageReader? = null
    private var mCaptureReader: ImageReader? = null

    private var mCall: CameraWrapCall? = null

    init {
        initCameraInfo()
        initCameraSize()
    }

    private fun initCameraInfo() {
        for (id in mCameraManager.cameraIdList) {
            val characteristics = mCameraManager.getCameraCharacteristics(id)
            val facings = characteristics.get(CameraCharacteristics.LENS_FACING) ?: continue
            when (facings) {
                CameraCharacteristics.LENS_FACING_FRONT -> {
                    CAMERA_FRONT = id
                    mCameras[CAMERA_FRONT] = characteristics
                }
                CameraCharacteristics.LENS_FACING_BACK -> {
                    CAMERA_BACK = id
                    mCameras[CAMERA_BACK] = characteristics
                }
                else -> {
                    CAMERA_EXTERNAL = id
                    mCameras[CAMERA_EXTERNAL] = characteristics
                }
            }
            this.facing = CAMERA_BACK
            val level = characteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL)
            if (level == null || level == CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY) break
        }
    }

    private fun initCameraSize() {
        val characteristics = mCameras[facing] ?: return
        val configs =
            characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP) ?: return
        val orientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION) ?: return
        Log.e("CameraWrap", "SENSOR_ORIENTATION : $orientation")
        val previewSizes: Array<Size> = configs.getOutputSizes(IMAGE_READER_FORMAT) ?: return
        val defaultRatio = mDefaultCaptureSize.width * 1.0f / mDefaultCaptureSize.height
        var sameRatioSize: Size? = null
        val threshold = 0.001f
        var supportDefaultSize = false
        for (size in previewSizes) {
            val ratio = size.width * 1.0f / size.height
            if (abs(ratio - defaultRatio) < threshold) {
                sameRatioSize = size
            }
            if (mDefaultPreviewSize.width == size.width && mDefaultPreviewSize.height == size.height) {
                supportDefaultSize = true
                break
            }
        }
        if (supportDefaultSize) {
            mPreviewSize = mDefaultPreviewSize
        } else if (sameRatioSize != null) {
            mPreviewSize = sameRatioSize
        }
        mCaptureSize = mPreviewSize
    }

    private fun initHandler() {
        mHandlerThread = HandlerThread("camera2")
        mHandlerThread?.start()
        mHandler = Handler(mHandlerThread?.looper ?: return)
    }
}