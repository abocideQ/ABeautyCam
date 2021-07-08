package lin.abcdq.vdmake.view.recyclerview.manager

import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager

class StaggeredManager : StaggeredGridLayoutManager {

    constructor(spanCount: Int) : super(spanCount, VERTICAL)

    constructor(spanCount: Int, vertical: Boolean) : super(spanCount, VERTICAL) {
        orientation = if (vertical) VERTICAL else HORIZONTAL
    }

    override fun onLayoutChildren(recycler: RecyclerView.Recycler?, state: RecyclerView.State?) {
        try {
            super.onLayoutChildren(recycler, state)
        } catch (e: IndexOutOfBoundsException) {
            e.printStackTrace()
        }
    }
}