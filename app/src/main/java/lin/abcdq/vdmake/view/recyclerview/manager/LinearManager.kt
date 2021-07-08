package lin.abcdq.vdmake.view.recyclerview.manager

import android.content.Context
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class LinearManager : LinearLayoutManager {

    constructor(context: Context) : super(context)

    constructor(context: Context, vertical: Boolean) : super(context) {
        orientation = if (vertical) VERTICAL else HORIZONTAL
    }

    override fun onLayoutChildren(
        recycler: RecyclerView.Recycler?,
        state: RecyclerView.State?
    ) {
        try {
            super.onLayoutChildren(recycler, state)
        } catch (e: IndexOutOfBoundsException) {
            e.printStackTrace()
        }
    }

}