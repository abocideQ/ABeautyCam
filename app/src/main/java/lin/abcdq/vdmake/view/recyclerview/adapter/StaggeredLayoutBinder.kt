package lin.abcdq.vdmake.view.recyclerview.adapter

interface StaggeredLayoutBinder : LinearLayoutBinder {
    fun spanFull(position: Int): Boolean
}