package lin.abcdq.vdmake

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.MotionEvent
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import lin.abcdq.vd.camera.VdCamera
import lin.abcdq.vd.record.VdRecord
import lin.abcdq.vdmake.utils.VdUIHelper

class CameraActivity : AppCompatActivity() {

    companion object {
        fun startActivity(context: Activity) {
            val intent = Intent(context, CameraActivity::class.java)
            context.startActivity(intent)
        }
    }

    private lateinit var mRatioView: TextView
    private lateinit var mSizeView: ImageView
    private lateinit var mFaceView: ImageView
    private lateinit var mShotView: ImageView
    private var mSizePosition = 0

    private lateinit var mContainer: LinearLayout
    private lateinit var mGlSurface: GLSurfaceView
    private lateinit var mCamera: VdCamera
    private var mInit = false
    private lateinit var mRecord: VdRecord

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        VdUIHelper.translucent(this.window)
        setContentView(R.layout.activity_camera)
        initClick()
        initCamera()
    }

    override fun onResume() {
        super.onResume()
        if (mInit) mCamera.onOpen()
        mInit = true
    }

    override fun onPause() {
        super.onPause()
        mCamera.onStop()
    }

    override fun onDestroy() {
        super.onDestroy()
        mCamera.onRelease()
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun initClick() {
        mContainer = findViewById(R.id.ll_container)
        mSizeView = findViewById(R.id.iv_size)
        mFaceView = findViewById(R.id.iv_face)
        mShotView = findViewById(R.id.iv_shot)
        mSizeView.setOnClickListener {
            mCamera.onSize()
            if (mSizePosition == -1) {
                for (size in mCamera.onSizes() ?: return@setOnClickListener) {
                    if (size.width == mCamera.onSize()?.width && size.height == mCamera.onSize()?.height) {
                        break
                    }
                    mSizePosition++
                }
            }
            if (mSizePosition + 1 >= mCamera.onSizes()?.size ?: 0 || mSizePosition < 0) {
                mSizePosition = 0
            } else {
                mSizePosition++
            }
            mCamera.onSize(
                mCamera.onSizes()?.get(mSizePosition) ?: return@setOnClickListener
            )
            resize()
        }
        mFaceView.setOnClickListener {
            mCamera.onSwitch()
        }
        mShotView.setOnTouchListener { _, event ->
            when (event?.action) {
                MotionEvent.ACTION_DOWN -> {
                    mRecord.onStart()
                    mShotView.setImageResource(R.drawable.camera_shotting)
                }
                MotionEvent.ACTION_UP -> {
                    mRecord.onStop()
                    mShotView.setImageResource(R.drawable.camera_shotted)
                }
                MotionEvent.ACTION_CANCEL -> {
                    mRecord.onStop()
                    mShotView.setImageResource(R.drawable.camera_shotted)
                }
            }
            true
        }
    }

    private fun initCamera() {
        mGlSurface = GLSurfaceView(this)
        mContainer.addView(mGlSurface)
        mCamera = VdCamera(this, 1)
        mCamera.setSurface(mGlSurface)
        resize()
        val out = obbDir.absolutePath + "/Record/"
        val size = mCamera.onSize() ?: return
        val fps = 30
        val rate = (size.width * size.height * fps * 0.3).toLong()
        mRecord.onSource(out, size.width, size.height, rate, fps)
    }

    @SuppressLint("SetTextI18n")
    private fun resize() {
        val size = mCamera.onSize() ?: return
        val ration: Float = size.width.toFloat() / size.height.toFloat()
        val width = VdUIHelper.screenWidth(this)
        val height: Int = (width * ration).toInt()
        val layout = mContainer.layoutParams
        layout.height = height
        mContainer.layoutParams = layout
        mRatioView = findViewById(R.id.tv_ratio)
        mRatioView.text = "${size.width}:${size.height}"
    }
}