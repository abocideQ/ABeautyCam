package lin.abcdq.vdmake.view.recyclerview.decoration

import android.graphics.Rect
import android.util.Log
import android.view.View
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager

class StaggeredDecoration : RecyclerView.ItemDecoration {

    private var mSpace = 0

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

        var spanCount = (parent.layoutManager as StaggeredGridLayoutManager).spanCount

        var holder = parent.findContainingViewHolder(view)
        var lp = holder?.itemView?.layoutParams as StaggeredGridLayoutManager.LayoutParams
        var spanIndex = lp.spanIndex// spanCount=3 spanIndex = 0 1 2
        if (lp.isFullSpan) {
            left = 0
            right = 0
            top = 0
            bottom = 0
        } else {
            when {
                spanIndex == 0 -> {
                    left = mSpace
                    right = mSpace / 2
                }
                spanIndex % (spanCount - 1) == 0 -> {
                    left = mSpace / 2
                    right = mSpace
                }
                else -> {
                    left = mSpace / 2
                    right = mSpace / 2
                }
            }
            top = 0
            bottom = mSpace
        }
        outRect.left = left
        outRect.top = top
        outRect.right = right
        outRect.bottom = bottom
    }
}