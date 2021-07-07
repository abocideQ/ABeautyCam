package lin.abcdq.vdmake

import android.Manifest
import android.content.pm.PackageManager
import android.content.pm.PermissionInfo
import android.opengl.GLSurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.RelativeLayout
import androidx.core.app.ActivityCompat
import lin.abcdq.vd.camera.VdCamera
import java.security.Permission
import java.security.Permissions

//VideoDecodeMake
class MainActivity : AppCompatActivity() {

    private val mPermissions = arrayOf(
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.CAMERA
    )

    private lateinit var mCamera: VdCamera
    private lateinit var mContainer: RelativeLayout
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        init()
    }

    private fun init() {
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
        if (check) {
            mContainer = findViewById(R.id.rl_container)
            val glSurfaceView = GLSurfaceView(this)
            mContainer.addView(glSurfaceView)
            mCamera = VdCamera(this, 1)
            mCamera.setSurface(glSurfaceView)
            mContainer.setOnClickListener {
                mCamera.onSwitch()
            }
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode != 10017) return
        init()
    }
}