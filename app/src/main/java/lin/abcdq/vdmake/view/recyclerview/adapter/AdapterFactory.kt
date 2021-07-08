package lin.abcdq.vdmake.view.recyclerview.adapter

import android.content.Context
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager

class AdapterFactory {

    companion object {

        fun adapterLinear(
            context: Context,
            list: List<Any>,
            binder: LinearLayoutBinder
        ): RecyclerView.Adapter<RecyclerView.ViewHolder> {
            return object : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

                override fun getItemViewType(position: Int): Int {
                    return binder.layout(position)
                }

                override fun onCreateViewHolder(
                    parent: ViewGroup,
                    viewType: Int
                ): RecyclerView.ViewHolder {
                    return object : RecyclerView.ViewHolder(
                        LayoutInflater.from(context).inflate(viewType, null)
                    ) {}
                }

                override fun getItemCount(): Int {
                    return list.size
                }

                override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
                    binder.onBindViewHolder(holder.itemViewType, holder, holder.itemView)
                }
            }
        }

        fun adapterGrid(
            context: Context,
            list: List<Any>,
            binder: GridLayoutBinder
        ): RecyclerView.Adapter<RecyclerView.ViewHolder> {
            return object : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

                override fun getItemViewType(position: Int): Int {
                    return binder.layout(position)
                }

                override fun onCreateViewHolder(
                    parent: ViewGroup,
                    viewType: Int
                ): RecyclerView.ViewHolder {
                    return object : RecyclerView.ViewHolder(
                        LayoutInflater.from(context).inflate(viewType, null)
                    ) {}
                }

                override fun onAttachedToRecyclerView(recyclerView: RecyclerView) {
                    super.onAttachedToRecyclerView(recyclerView)
                    var manager = recyclerView.layoutManager
                    if (manager !is GridLayoutManager) return
                    manager.spanSizeLookup = object : GridLayoutManager.SpanSizeLookup() {
                        override fun getSpanSize(position: Int): Int {
                            return binder.spanSize(position)
                        }
                    }
                }

                override fun getItemCount(): Int {
                    return list.size
                }

                override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
                    binder.onBindViewHolder(holder.itemViewType, holder, holder.itemView)
                }
            }
        }

        fun adapterStaggered(
            context: Context,
            list: List<Any>,
            binder: StaggeredLayoutBinder
        ): RecyclerView.Adapter<RecyclerView.ViewHolder> {
            return object : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

                override fun getItemViewType(position: Int): Int {
                    return binder.layout(position)
                }

                override fun onCreateViewHolder(
                    parent: ViewGroup,
                    viewType: Int
                ): RecyclerView.ViewHolder {
                    return object : RecyclerView.ViewHolder(
                        LayoutInflater.from(context).inflate(viewType, null)
                    ) {}
                }

                override fun onViewAttachedToWindow(holder: RecyclerView.ViewHolder) {
                    super.onViewAttachedToWindow(holder)
                    var lp = holder.itemView.layoutParams
                    if (lp !is StaggeredGridLayoutManager.LayoutParams) return
                    lp.isFullSpan = binder.spanFull(holder.adapterPosition)
                    holder.itemView.layoutParams = lp
                }

                override fun getItemCount(): Int {
                    return list.size
                }

                override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
                    binder.onBindViewHolder(holder.itemViewType, holder, holder.itemView)
                }
            }
        }
    }
}