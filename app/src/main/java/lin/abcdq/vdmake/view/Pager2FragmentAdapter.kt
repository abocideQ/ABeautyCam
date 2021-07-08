package lin.abcdq.vdmake.view

import androidx.collection.LongSparseArray
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.fragment.app.FragmentManager
import androidx.lifecycle.Lifecycle
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.adapter.FragmentStateAdapter
import androidx.viewpager2.widget.ViewPager2
import com.google.android.material.tabs.TabLayout
import com.google.android.material.tabs.TabLayoutMediator


class Pager2FragmentAdapter : FragmentStateAdapter {

    constructor(context: FragmentActivity) : super(
        context.supportFragmentManager,
        context.lifecycle
    ) {
        mFragmentManager = context.supportFragmentManager
    }

    constructor(context: Fragment) : super(
        context.childFragmentManager,
        context.lifecycle
    ) {
        mFragmentManager = context.childFragmentManager
    }

    constructor(fragmentManager: FragmentManager, lifecycle: Lifecycle) : super(
        fragmentManager,
        lifecycle
    ) {
        mFragmentManager = fragmentManager
    }

    private var mFragmentManager: FragmentManager
    private lateinit var mCreateListener: CreateListener
    private var mTabTexts = listOf<Any>()

    fun set(
        viewPager: ViewPager2,
        tabLayout: TabLayout,
        tabTexts: List<Any>,
        listener: CreateListener
    ) {
        TabLayoutMediator(tabLayout, viewPager) { tab, position ->
            tab.text = "${mTabTexts[position]}"
        }.attach()
        mTabTexts = tabTexts
        mCreateListener = listener
        notifyDataSetChanged()
    }

    fun set(
        size: Int,
        listener: CreateListener
    ) {
        mTabTexts = listOf(arrayOfNulls<Any>(size))
        mCreateListener = listener
        notifyDataSetChanged()
    }

    fun fragment(position: Int): Fragment? {
        return findByTag(position) ?: findByField(position)
    }

    fun setPrefetchEnable(view: ViewPager2, enable: Boolean) {
        (view.getChildAt(0) as RecyclerView).layoutManager?.isItemPrefetchEnabled = enable
    }

    override fun getItemCount(): Int {
        return mTabTexts.size
    }

    override fun createFragment(position: Int): Fragment {
        return mCreateListener.onCreate(position)
    }

    private fun findByTag(position: Int): Fragment? {
        return mFragmentManager.findFragmentByTag("f$position")
    }

    private fun findByField(position: Int): Fragment? {
        val field = this.javaClass.superclass?.getDeclaredField("mFragments")
        field?.isAccessible = true
        val any = field?.get(this) ?: return null
        if (any !is LongSparseArray<*>) return null
        if (position >= any.size()) return null
        return any[position.toLong()] as Fragment?
    }

    interface CreateListener {
        fun onCreate(position: Int): Fragment
    }
}