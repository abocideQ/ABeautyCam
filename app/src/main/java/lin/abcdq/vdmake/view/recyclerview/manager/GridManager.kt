package lin.abcdq.vdmake.view.recyclerview.manager

import android.content.Context
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView

class GridManager : GridLayoutManager {

    constructor(context: Context, spanCount: Int) : super(context, spanCount)

    constructor(context: Context, spanCount: Int, vertical: Boolean) : super(context, spanCount) {
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