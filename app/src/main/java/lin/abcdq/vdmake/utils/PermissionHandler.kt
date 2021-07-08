package lin.abcdq.vdmake.utils

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment

class PermissionHandler : Fragment() {

    private val mPermissions = arrayOf(
        Manifest.permission.CAMERA,
        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.READ_EXTERNAL_STORAGE
    )

    private val mRequestCode = 111
    private var mResponse: PermissionsResponse? = null

    fun setCall(response: PermissionsResponse) {
        mResponse = response
    }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        if (mResponse == null) return
        if (ContextCompat.checkSelfPermission(
                context,
                mPermissions[0]
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermissions(mPermissions, mRequestCode)
        } else mResponse?.response(true)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode != mRequestCode) return
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            mResponse?.response(true)
        } else if (grantResults[0] == PackageManager.PERMISSION_DENIED) {
            mResponse?.response(false)
        }
    }

    interface PermissionsResponse {
        fun response(all: Boolean)
    }
}