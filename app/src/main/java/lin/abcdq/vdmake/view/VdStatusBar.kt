package lin.abcdq.vdmake.view

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.Resources
import android.util.AttributeSet
import android.view.View

class VdStatusBar : View {

    companion object {
        private var mStatusHeight = 0
    }

    constructor(context: Context) : super(context)
    constructor(context: Context, attr: AttributeSet) : super(context, attr)
    constructor(context: Context, attr: AttributeSet, a: Int) : super(context, attr, a)

    private var mWidthMeasureSpec = 0

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        mWidthMeasureSpec = widthMeasureSpec
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        setMeasuredDimension(measureW(), measureH())
    }

    private fun measureW(): Int {
        return MeasureSpec.getSize(mWidthMeasureSpec)
    }

    private fun measureH(): Int {
        if (mStatusHeight == 0) mStatusHeight = statusHeight()
        return mStatusHeight
    }

    @SuppressLint("PrivateApi")
    private fun statusHeight(): Int {
        var height = 0
        if (height <= 0) {
            val id = resources.getIdentifier("status_bar_height", "dimen", "android")
            if (id > 0) height = resources.getDimensionPixelSize(id)
        }
        if (height <= 0) {
            try {
                val clazz = Class.forName("com.android.internal.R\$dimen")
                val ob = clazz.newInstance() ?: null
                val fd = clazz.getField("status_bar_height")
                val id = fd.get(ob ?: 0)?.toString()?.toInt() ?: 0
                if (id > 0) height = resources.getDimensionPixelSize(id)
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
        if (height <= 0) {
            val defaultDp = 25
            val defaultHeight = defaultDp * Resources.getSystem().displayMetrics.density + 0.5f
            height = defaultHeight.toInt()
        }
        return height
    }
}