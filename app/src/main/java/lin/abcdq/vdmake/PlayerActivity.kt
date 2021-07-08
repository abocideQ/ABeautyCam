package lin.abcdq.vdmake

import android.app.Activity
import android.content.Intent
import android.opengl.GLSurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.ViewGroup
import android.widget.LinearLayout
import lin.abcdq.vd.player.VdPlayer
import lin.abcdq.vdmake.utils.VdUIHelper

class PlayerActivity : AppCompatActivity() {

    companion object {
        fun startActivity(context: Activity) {
            val intent = Intent(context, PlayerActivity::class.java)
            context.startActivity(intent)
        }
    }

    private lateinit var mContainer: LinearLayout
    private lateinit var mGlSurface: GLSurfaceView
    private lateinit var mPlayer: VdPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        VdUIHelper.translucent(this.window)
        setContentView(R.layout.activity_player)
        mContainer = findViewById(R.id.ll_container)
        initVideo()
    }

    override fun onResume() {
        super.onResume()
        mPlayer.onPlay()
    }

    override fun onPause() {
        super.onPause()
        mPlayer.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        mPlayer.onRelease()
    }

    private fun initVideo() {
        mGlSurface = GLSurfaceView(this)
        mContainer.addView(mGlSurface)
        mPlayer = VdPlayer()
        mPlayer.onSetSurface(mGlSurface)
        mPlayer.onSource("http://vfx.mtime.cn/Video/2019/03/21/mp4/190321153853126488.mp4")
        mPlayer.onPlay()
    }
}