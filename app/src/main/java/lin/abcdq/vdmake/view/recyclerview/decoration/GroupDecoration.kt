package lin.abcdq.vdmake.view.recyclerview.decoration

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.view.View
import androidx.core.view.drawToBitmap
import androidx.recyclerview.widget.RecyclerView
import kotlin.math.max

class GroupDecoration(listener: Listener) : RecyclerView.ItemDecoration() {

    private var mPaint: Paint = Paint()
    private var mSize = 0
    private var mListener: Listener = listener
    private var mCaches = HashMap<Int, Bitmap>()

    init {
        mPaint.isAntiAlias = true
        mSize = mListener.height()
    }

    override fun getItemOffsets(
        outRect: Rect,
        view: View,
        parent: RecyclerView,
        state: RecyclerView.State
    ) {
        val position = parent.getChildAdapterPosition(view)
        if (mListener.group(position)) outRect.top = mSize
        else outRect.top = 0
    }

    override fun onDraw(canvas: Canvas, parent: RecyclerView, state: RecyclerView.State) {
        super.onDraw(canvas, parent, state)
        for (i in 0..parent.childCount) {
            var view: View? = parent.getChildAt(i) ?: return
            var position = parent.getChildAdapterPosition(view!!)
            if (mListener.group(position)) {
                if (!mCaches.containsKey(position)) {
                    var draw = mListener.view(position)
                    mCaches[position] = view2B(draw, parent)
                }
                mCaches[position]?.let {
                    canvas.drawBitmap(
                        it,
                        0F,
                        (view.top - mSize).toFloat(),
                        mPaint
                    )
                }
            } else {
                continue
            }
        }
    }

    override fun onDrawOver(canvas: Canvas, parent: RecyclerView, state: RecyclerView.State) {
        super.onDrawOver(canvas, parent, state)
        for (i in 0..parent.childCount) {
            var view: View? = parent.getChildAt(i) ?: return
            var position = parent.getChildAdapterPosition(view!!)
            if (mListener.group(position)) {
                var bottom = view.bottom.toFloat()
                var top = max(mSize, view.top).toFloat()
                if (position + 1 < state.itemCount && bottom < top) top = bottom
                if (!mCaches.containsKey(position)) {
                    var draw = mListener.view(position)
                    mCaches[position] = view2B(draw, parent)
                }
                mCaches[position]?.let { canvas.drawBitmap(it, 0F, top - mSize, mPaint) }
            } else {
                continue
            }
        }
    }

    private fun view2B(view: View, parent: RecyclerView?): Bitmap {
        var width = parent?.width ?: 0
        val widthSpec = View.MeasureSpec.makeMeasureSpec(width, View.MeasureSpec.EXACTLY)
        val heightSpec = View.MeasureSpec.makeMeasureSpec(mSize, View.MeasureSpec.EXACTLY)
        view.measure(widthSpec, heightSpec)
        view.layout(0, 0, width, mSize)
        val bitmap = Bitmap.createBitmap(width, mSize, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        view.draw(canvas)
        return view.drawToBitmap()
    }

    interface Listener {
        fun height(): Int
        fun group(position: Int): Boolean
        fun view(position: Int): View
    }
}