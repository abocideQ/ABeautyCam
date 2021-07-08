package lin.abcdq.vdmake.view

import android.annotation.SuppressLint
import android.content.Context
import android.media.Image
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.Toast
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.widget.ViewPager2
import com.bumptech.glide.Glide
import lin.abcdq.vdmake.R
import org.xmlpull.v1.XmlPullParser
import org.xmlpull.v1.XmlPullParserFactory
import java.util.concurrent.Executors

class VdBanner : FrameLayout {

    constructor(context: Context) : super(context) {
        init(context)
    }

    constructor(context: Context, attr: AttributeSet) : super(context, attr) {
        init(context)
    }

    constructor(context: Context, attr: AttributeSet, a: Int) : super(context, attr, a) {
        init(context)
    }

    private var mViewPager: ViewPager2? = null
    private var mAdapter: RecyclerView.Adapter<RecyclerView.ViewHolder>? = null
    private var mList = ArrayList<Any>()

    fun setData(list: ArrayList<Any>) {
        if (mList.isNotEmpty()) return
        mList.clear()
        mAdapter?.notifyDataSetChanged()
        mList.addAll(list)
        mAdapter?.notifyDataSetChanged()
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun init(context: Context) {
        mViewPager = ViewPager2(context)
        addView(mViewPager)
        mAdapter = object : RecyclerView.Adapter<RecyclerView.ViewHolder>() {
            override fun onCreateViewHolder(
                parent: ViewGroup,
                viewType: Int
            ): RecyclerView.ViewHolder {
                return object : RecyclerView.ViewHolder(
                    LayoutInflater.from(context).inflate(R.layout.vd_banner, parent, false)
                ) {}
            }

            override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
                val view = holder.itemView as ViewGroup
                val imageView: ImageView = view.findViewById(R.id.iv_banner)
                Glide.with(context).load("${mList[holder.adapterPosition]}").centerCrop()
                    .into(imageView)
                imageView.setOnClickListener {
                    Toast.makeText(context, "你再点试试看？", Toast.LENGTH_SHORT).show()
                }
            }

            override fun getItemCount(): Int {
                return mList.size
            }
        }
        mViewPager?.adapter = mAdapter
        runLoop()
    }

    private val mSingleThread = Executors.newSingleThreadExecutor()
    private var mInterrupt = false

    private fun runLoop() {
        mSingleThread.execute {
            while (!mInterrupt) {
                Thread.sleep(5000)
                if (mViewPager == null || mViewPager?.currentItem == null) {
                    break
                } else if (mViewPager?.currentItem ?: 0 >= mList.size - 1) {
                    mViewPager?.currentItem = 0
                } else {
                    mViewPager?.currentItem = (mViewPager?.currentItem ?: 0) + 1
                }
            }
        }
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        mInterrupt = true
    }
}