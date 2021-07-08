package lin.abcdq.vdmake.view.recyclerview.decoration

import android.graphics.Rect
import android.view.View
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class LinearDecoration : RecyclerView.ItemDecoration {

    private var mBorder = true
    private var mOrientation = LinearLayoutManager.VERTICAL
    private var mSpace = 0

    fun setBorder(border: Boolean) {
        mBorder = border
    }

    fun setOrientation(orientation: Int) {
        mOrientation = orientation
    }

    constructor(space: Int) : super() {
        mSpace = space
    }

    override fun getItemOffsets(
        outRect: Rect,
        view: View,
        parent: RecyclerView,
        state: RecyclerView.State
    ) {
        super.getItemOffsets(outRect, view, parent, state)
        var left: Int
        var top: Int
        var right: Int
        var bottom: Int
        var position = parent.getChildAdapterPosition(view)
        if (mOrientation == LinearLayoutManager.VERTICAL) {
            if (mBorder) {
                left = mSpace
                right = mSpace
                top = if (position == 0) mSpace else 0
                bottom = mSpace
            } else {
                left = 0
                right = 0
                top = if (position == 0) 0 else mSpace
                bottom = 0
            }
        } else {
            if (mBorder) {
                left = if (position == 0) mSpace else 0
                right = mSpace
                top = mSpace
                bottom = mSpace
            } else {
                left = if (position == 0) 0 else mSpace
                right = 0
                top = 0
                bottom = 0
            }
        }
        outRect.left = left
        outRect.top = top
        outRect.right = right
        outRect.bottom = bottom
    }

}