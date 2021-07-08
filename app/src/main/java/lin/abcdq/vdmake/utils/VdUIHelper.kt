package lin.abcdq.vdmake.utils

import android.content.Context
import android.content.res.Resources
import android.graphics.Color
import android.view.Window
import android.view.WindowInsetsController
import android.view.WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS


object VdUIHelper {

    fun dp2Px(dp: Int): Int {
        return ((dp * Resources.getSystem().displayMetrics.density + 0.5f).toInt())
    }

    fun screenHeight(context: Context): Int {
        return context.resources.displayMetrics.heightPixels
    }

    fun screenWidth(context: Context): Int {
        return context.resources.displayMetrics.widthPixels
    }

    fun translucent(window: Window) {
        val controller: WindowInsetsController = window.decorView.windowInsetsController ?: return
        controller.setSystemBarsAppearance(
            APPEARANCE_LIGHT_STATUS_BARS,
            APPEARANCE_LIGHT_STATUS_BARS
        )
        window.setDecorFitsSystemWindows(false)
        window.navigationBarColor = Color.TRANSPARENT
        window.statusBarColor = Color.TRANSPARENT
    }
}