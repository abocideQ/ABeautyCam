package lin.abcdq.vdmake

import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import androidx.fragment.app.Fragment
import lin.abcdq.vd.camera.VdCamera
import lin.abcdq.vd.player.VdPlayer
import lin.abcdq.vdmake.utils.VdUIHelper

class VdFragment(position: Int) : Fragment() {

    companion object {
        fun instance(position: Int): VdFragment {
            return VdFragment(position)
        }
    }

    private lateinit var mContainer: LinearLayout

    //1.camera 2.ffmpeg
    private var mPosition: Int = position

    override fun onCreateView(inflater: LayoutInflater, con: ViewGroup?, save: Bundle?): View? {
        return inflater.inflate(R.layout.fragment_main, con)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        mContainer = view.findViewById(R.id.ll_container)
        when (mPosition) {
            1 -> initCamera()
            2 -> initVideo()
        }
    }

    private fun initCamera() {
        val glSurfaceView = GLSurfaceView(context)
        mContainer.addView(
            glSurfaceView,
            ViewGroup.LayoutParams.MATCH_PARENT,
            VdUIHelper.dp2Px(500)
        )
        val camera = VdCamera(context ?: return)
        camera.setSurface(glSurfaceView)
        glSurfaceView.setOnClickListener {
            camera.onSwitch()
        }
    }

    private fun initVideo() {
        val glSurfaceView = GLSurfaceView(context)
        mContainer.addView(
            glSurfaceView,
            ViewGroup.LayoutParams.MATCH_PARENT,
            VdUIHelper.dp2Px(500)
        )
        val player = VdPlayer()
        player.onSetSurface(glSurfaceView)
        player.onSource("http://vfx.mtime.cn/Video/2019/03/21/mp4/190321153853126488.mp4")
    }
}