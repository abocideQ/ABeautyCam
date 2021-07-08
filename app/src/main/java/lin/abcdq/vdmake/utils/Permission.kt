package lin.abcdq.vdmake.utils

import androidx.fragment.app.FragmentActivity
import lin.abcdq.vdmake.utils.PermissionHandler.PermissionsResponse


object Permission {

    @Volatile
    private var mPermission: PermissionHandler? = null

    private fun instance(): PermissionHandler? {
        if (mPermission == null) {
            synchronized(PermissionHandler::class.java) {
                if (mPermission == null) {
                    mPermission = PermissionHandler()
                }
            }
        }
        return mPermission
    }

    fun request(activity: FragmentActivity, response: PermissionsResponse) {
        instance()?.setCall(response)
        activity.supportFragmentManager.beginTransaction().add(instance() ?: return, "").commit()
    }

    fun clear(activity: FragmentActivity) {
        activity.supportFragmentManager.beginTransaction().remove(instance() ?: return).commit()
    }
}