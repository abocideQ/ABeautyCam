package lin.abcdq.vdmake

import android.os.Bundle
import android.view.View
import android.widget.ImageView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.widget.ViewPager2
import com.bumptech.glide.Glide
import com.google.android.material.tabs.TabLayout
import lin.abcdq.vdmake.view.Pager2FragmentAdapter
import lin.abcdq.vdmake.utils.Permission
import lin.abcdq.vdmake.utils.PermissionHandler
import lin.abcdq.vdmake.utils.VdUIHelper
import lin.abcdq.vdmake.view.VdBanner
import lin.abcdq.vdmake.view.recyclerview.adapter.AdapterFactory
import lin.abcdq.vdmake.view.recyclerview.adapter.GridLayoutBinder
import lin.abcdq.vdmake.view.recyclerview.decoration.GridDecoration
import lin.abcdq.vdmake.view.recyclerview.manager.GridManager

//VideoDecodeMake
class MainActivity : AppCompatActivity(), GridLayoutBinder {

    private val mItemImages = arrayOf(
        "https://gimg2.baidu.com/image_search/src=http%3A%2F%2F5b0988e595225.cdn.sohucs.com%2Fimages%2F20171107%2Fa8c06971a2b14494a86885f2feb2bd46.gif&refer=http%3A%2F%2F5b0988e595225.cdn.sohucs.com&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1628319577&t=4de1c7aff5bf7023878807cc730b7502",
        "https://gimg2.baidu.com/image_search/src=http%3A%2F%2Fimg0.ph.126.net%2F-dL-anG_WUfZ4RGBUZNjyg%3D%3D%2F798544508930078709.gif&refer=http%3A%2F%2Fimg0.ph.126.net&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1628319599&t=79717a7600683b7229e1c087cb1c2742",
        "https://gimg2.baidu.com/image_search/src=http%3A%2F%2Fdingyue.nosdn.127.net%2FuOykMLUM1skH%3DZCMuklxKlpFET3QhoBM65xLz9QxHqbzj1541416531260.gif&refer=http%3A%2F%2Fdingyue.nosdn.127.net&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1628318704&t=e47a833b95840ad6cf784e452de805f2",
        "https://gimg2.baidu.com/image_search/src=http%3A%2F%2F5b0988e595225.cdn.sohucs.com%2Fimages%2F20170731%2F410237c29fbe460ea3ad861d4cd9ded7.gif&refer=http%3A%2F%2F5b0988e595225.cdn.sohucs.com&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1628318768&t=83e69fe954df8a22571ba490f70bafc1"
    )
    private val mBannerImages = arrayOf(
        "https://t7.baidu.com/it/u=2531125946,3055766435&fm=193&f=GIF",
        "https://t7.baidu.com/it/u=1330338603,908538247&fm=193&f=GIF",
        "https://t7.baidu.com/it/u=657578767,2750473856&fm=193&f=GIF"
    )

    private lateinit var mRecyclerView: RecyclerView
    private val mAdapter by lazy {
        AdapterFactory.adapterGrid(this, mList, this)
    }
    private val mList = ArrayList<Any>()
    private val mListBanner = ArrayList<Any>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        VdUIHelper.translucent(this.window)
        setContentView(R.layout.activity_main)
        initData()
        Permission.request(this, object : PermissionHandler.PermissionsResponse {
            override fun response(all: Boolean) {
                init()
            }
        })
    }

    private fun initData() {
        for (string in mBannerImages) {
            mListBanner.add(string)
        }
        mList.add("banner")
        for (string in mItemImages) {
            mList.add(string)
        }
    }

    private fun init() {
        mRecyclerView = findViewById(R.id.rv_content)
        mRecyclerView.layoutManager = GridManager(this, 2)
        mRecyclerView.addItemDecoration(GridDecoration(VdUIHelper.dp2Px(8)))
        mRecyclerView.adapter = mAdapter
        mAdapter.notifyDataSetChanged()
    }

    override fun spanSize(position: Int): Int {
        if (position == 0) return 2
        return 1
    }

    override fun layout(position: Int): Int {
        if (position == 0) return R.layout.item_main_banner
        return R.layout.item_main_list
    }

    override fun onBindViewHolder(layout: Int, holder: RecyclerView.ViewHolder, view: View) {
        if (layout == R.layout.item_main_banner) {
            val banner: VdBanner = view.findViewById(R.id.vb_content)
            banner.setData(mListBanner)
        } else {
            val imageView: ImageView = view.findViewById(R.id.iv_icon)
            val textView: TextView = view.findViewById(R.id.tv_title)
            Glide.with(this).load(mList[holder.adapterPosition]).centerCrop().into(imageView)
            when (holder.adapterPosition) {
                1 -> {
                    textView.text = "摄像头"
                    holder.itemView.setOnClickListener { CameraActivity.startActivity(this) }
                }
                2 -> {
                    textView.text = "播放器"
                    holder.itemView.setOnClickListener { PlayerActivity.startActivity(this) }
                }
                3 -> {
                    textView.text = "工具籍"
                }
                4 -> {
                    textView.text = "关于"
                }
            }
        }
    }
}