package lin.abcdq.vdmake.view.recyclerview.decoration

import android.graphics.Rect
import android.view.View
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class GridDecoration : RecyclerView.ItemDecoration {

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
        var spanCount = (parent.layoutManager as GridLayoutManager).spanCount
        var spanSize =
            (parent.layoutManager as GridLayoutManager).spanSizeLookup.getSpanSize(position)

        var holder = parent.findContainingViewHolder(view)
        var lp = holder?.itemView?.layoutParams as GridLayoutManager.LayoutParams
        var spanIndex = lp.spanIndex// spanCount=3 spanIndex = 0 1 2
        if (mOrientation == LinearLayoutManager.VERTICAL) {
            if (mBorder) {
                when {
                    spanCount == spanSize -> {
                        left = mSpace
                        right = mSpace
                    }
                    spanIndex == 0 -> {//左
                        left = mSpace
                        right = mSpace / 2
                    }
                    spanIndex % (spanCount - 1) == 0 -> {//右
                        left = mSpace / 2
                        right = mSpace
                    }
                    else -> {//中
                        left = mSpace / 2
                        right = mSpace / 2
                    }
                }
                var over = false
                var topSpanCount = 0
                for (i in 0..position) {
                    topSpanCount += (parent.layoutManager as GridLayoutManager).spanSizeLookup.getSpanSize(
                        i
                    )
                    if (topSpanCount > spanCount) {
                        over = true
                        break
                    }
                }
                top = if (!over) mSpace else 0
                bottom = mSpace
            } else {
                when {
                    spanCount == spanSize -> {
                        left = 0
                        right = 0
                    }
                    spanIndex == 0 -> {//左
                        left = 0
                        right = mSpace / 2
                    }
                    spanIndex % (spanCount - 1) == 0 -> {//右
                        left = mSpace / 2
                        right = 0
                    }
                    else -> {//中
                        left = mSpace / 2
                        right = mSpace / 2
                    }
                }
                var over = false
                var topSpanCount = 0
                for (i in 0..position) {
                    topSpanCount += (parent.layoutManager as GridLayoutManager).spanSizeLookup.getSpanSize(
                        i
                    )
                    if (topSpanCount > spanCount) {
                        over = true
                        break
                    }
                }
                top = if (!over) 0 else mSpace
                bottom = 0
            }
        } else {
            if (mBorder) {
                when {
                    spanCount == spanSize -> {
                        top = mSpace
                        bottom = mSpace
                    }
                    spanIndex == 0 -> {//左
                        top = mSpace
                        bottom = mSpace / 2
                    }
                    spanIndex % (spanCount - 1) == 0 -> {//右
                        top = mSpace / 2
                        bottom = mSpace
                    }
                    else -> {//中
                        top = mSpace / 2
                        bottom = mSpace / 2
                    }
                }
                var over = false
                var topSpanCount = 0
                for (i in 0..position) {
                    topSpanCount += (parent.layoutManager as GridLayoutManager).spanSizeLookup.getSpanSize(
                        i
                    )
                    if (topSpanCount > spanCount) {
                        over = true
                        break
                    }
                }
                left = if (!over) mSpace else 0
                right = mSpace
            } else {
                when {
                    spanCount == spanSize -> {
                        top = 0
                        bottom = 0
                    }
                    spanIndex == 0 -> {//左
                        top = 0
                        bottom = mSpace / 2
                    }
                    spanIndex % (spanCount - 1) == 0 -> {//右
                        top = mSpace / 2
                        bottom = 0
                    }
                    else -> {//中
                        top = mSpace / 2
                        bottom = mSpace / 2
                    }
                }
                var over = false
                var topSpanCount = 0
                for (i in 0..position) {
                    topSpanCount += (parent.layoutManager as GridLayoutManager).spanSizeLookup.getSpanSize(
                        i
                    )
                    if (topSpanCount > spanCount) {
                        over = true
                        break
                    }
                }
                left = if (!over) 0 else mSpace
                right = 0
            }
        }
        outRect.left = left
        outRect.top = top
        outRect.right = right
        outRect.bottom = bottom
    }

}