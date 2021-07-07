package lin.abcdq.vdmake

import android.Manifest
import android.content.pm.PackageManager
import android.content.res.Resources
import android.graphics.Color
import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Bundle
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.LinearLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import lin.abcdq.vd.camera.VdCamera
import lin.abcdq.vd.player.VdPlayer

//VideoDecodeMake
class MainActivity : AppCompatActivity() {

    private lateinit var mCamera: VdCamera
    private lateinit var mContainer: LinearLayout
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        translucent()
        setContentView(R.layout.activity_main)
        if (check()) init()
    }

    private fun init() {
        //摄像头
        mContainer = findViewById(R.id.rl_container)
        val glSurfaceView1 = GLSurfaceView(this)
        mContainer.addView(glSurfaceView1, ViewGroup.LayoutParams.MATCH_PARENT, dp2Px(500))
        mCamera = VdCamera(this, 1)
        mCamera.setSurface(glSurfaceView1)
        glSurfaceView1.setOnClickListener {
            mCamera.onSwitch()
        }
        //视频
        val glSurfaceView2 = GLSurfaceView(this)
        mContainer.addView(glSurfaceView2, ViewGroup.LayoutParams.MATCH_PARENT, dp2Px(500))
        val player = VdPlayer()
        player.onSetSurface(glSurfaceView2)
        player.onSource("http://vfx.mtime.cn/Video/2019/03/21/mp4/190321153853126488.mp4")
    }

    //权限
    private val mPermissions = arrayOf(
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.CAMERA
    )

    private fun check(): Boolean {
        var check = true
        for (permission in mPermissions) {
            if (ActivityCompat.checkSelfPermission(
                    this,
                    permission
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                check = false
                requestPermissions(mPermissions, 10017)
            }
        }
        return check
    }

    override fun onRequestPermissionsResult(code: Int, p: Array<out String>, g: IntArray) {
        super.onRequestPermissionsResult(code, p, g)
        if (code != 10017) return
        if (check()) init()
    }

    //dp转换
    private fun dp2Px(dp: Int): Int {
        return ((dp * Resources.getSystem().displayMetrics.density + 0.5f).toInt())
    }

    //状态栏
    private fun translucent() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)
            window.statusBarColor = Color.TRANSPARENT
        }
    }
}