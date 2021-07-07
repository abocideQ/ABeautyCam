package lin.abcdq.vd.player

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class VdPlayer : GLSurfaceView.Renderer {

    fun onSetSurface(surface: GLSurfaceView) {
        surface.setEGLContextClientVersion(3)
        surface.setRenderer(this)
    }

    fun onInfoPrint() {
        native_vdPlayer_onInfoPrint()
    }

    fun onSource(url: String) {
        native_vdPlayer_onSource(url)
    }

    fun onSeekTo(percent: Int) {
        native_vdPlayer_onSeekTo(percent)
    }

    fun onPlay() {
        native_vdPlayer_onPlay()
    }

    fun onPause() {
        native_vdPlayer_onPause()
    }

    fun onStop() {
        native_vdPlayer_onStop()
    }

    fun onRelease() {
        native_vdPlayer_onRelease()
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        native_vdPlayer_onSurfaceCreated()
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        native_vdPlayer_onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        native_vdPlayer_onDrawFrame()
    }

    init {
        System.loadLibrary("vd_make")
    }

    private external fun native_vdPlayer_onInfoPrint()

    private external fun native_vdPlayer_onSource(url: String)

    private external fun native_vdPlayer_onPlay()

    private external fun native_vdPlayer_onSeekTo(percent: Int)

    private external fun native_vdPlayer_onPause()

    private external fun native_vdPlayer_onStop()

    private external fun native_vdPlayer_onRelease()

    private external fun native_vdPlayer_onSurfaceCreated()

    private external fun native_vdPlayer_onSurfaceChanged(width: Int, height: Int)

    private external fun native_vdPlayer_onDrawFrame()
}