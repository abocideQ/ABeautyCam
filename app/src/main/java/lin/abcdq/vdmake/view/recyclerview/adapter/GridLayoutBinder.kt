package lin.abcdq.vdmake.view.recyclerview.adapter

interface GridLayoutBinder : LinearLayoutBinder {
    fun spanSize(position: Int): Int
}