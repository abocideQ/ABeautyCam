package lin.abcdq.vd.facepp

import android.content.Context
import android.util.Log
import com.megvii.facepp.multi.sdk.*
import com.megvii.facepp.multi.sdk.FaceDetectApi.FaceppConfig
import com.megvii.facepp.multi.sdk.FaceDetectApi.FaceppConfig.DETECTION_MODE_DETECT
import com.megvii.licensemanager.sdk.LicenseManager
import com.megvii.licensemanager.sdk.LicenseManager.TakeLicenseCallback
import lin.abcdq.vd.camera.util.CAO
import lin.abcdq.vd.facepp.utils.ConUtil
import lin.abcdq.vd.facepp.utils.Util
import java.io.File
import java.util.concurrent.Executors


class FacePlus {

    private val mThread = Executors.newSingleThreadExecutor()
    private val facePlusUrl = "https://api-cn.faceplusplus.com/sdk/v3/auth"
    private val facePlusKey = "1_2bU_3GrAkmjNSjzUozWzxTxSPwFdEN"
    private val facePlusSecret = "KB8m-ksomUaBDgkKs-ZEb5RSd6CW0Ov9"

    fun auth(context: Context) {
        mThread.execute {
            CAO.copyAssetsDirToSDCard(context, "facepp", context.obbDir.absolutePath)
            val modelBytes =
                File(context.obbDir.absolutePath + "/facepp/megviifacepp_model").readBytes()
            val ability = FaceppApi.getInstance().getModelAbility(modelBytes)
            val authManager = FacePPMultiAuthManager(ability)
            val licenseManager = LicenseManager(context)
            licenseManager.registerLicenseManager(authManager)
            val uuid: String = Util.getUUIDString(context)
            licenseManager.takeLicenseFromNetwork(facePlusUrl,
                uuid,
                facePlusKey,
                facePlusSecret,
                "1",
                object : TakeLicenseCallback {
                    override fun onSuccess() {
                        FaceppApi.getInstance().initHandle(modelBytes) //初始化facepp sdk，加载模型
                        FaceDetectApi.getInstance().initFaceDetect() // 初始化人脸检测
                        DLmkDetectApi.getInstance().initDLmkDetect() //初始化稠密点检测
                        val config = FaceppConfig()
                        config.face_confidence_filter = 0.6f
                        config.detectionMode = DETECTION_MODE_DETECT
                        FaceDetectApi.getInstance().faceppConfig = config
                    }

                    override fun onFailed(i: Int, bytes: ByteArray) {
                        Log.e("licenseManager", "takeLicenseFromNetwork : onFailed")
                    }
                })
        }
    }

    fun face(bytes: ByteArray, w: Int, h: Int, listener: FacePlusListener) {
        mThread.execute {
            val facePPImage = FacePPImage.Builder()
                .setData(bytes)
                .setWidth(w)
                .setHeight(h)
                .setMode(FacePPImage.IMAGE_MODE_NV21)
                .setRotation(FacePPImage.FACE_UP).build()
            val faces = FaceDetectApi.getInstance().detectFace(facePPImage)
            if (faces == null) {
                Log.e("aaaaaaaaaaaaaaaaaaaaaaaaaa", "|2222222");
                listener.onFace(null)
                return@execute
            }
            Log.e("aaaaaaaaaaaaaaaaaaaaaaaaaa", "|3333333");
            for (face in faces) {
                FaceDetectApi.getInstance().getRect(face, true) //获取人脸框
                FaceDetectApi.getInstance().getLandmark(face, FaceDetectApi.LMK_84, true) //获取人脸关键点
//             FaceDetectApi.getInstance().getExpression(face);//获取表情
//            FaceDetectApi.getInstance().getEyeGaze(face) //获取视线
//            FaceDetectApi.getInstance().getPose3D(face) //获取3dpose
//            FaceDetectApi.getInstance().getAgeGender(face) //年龄性别
//            FaceDetectApi.getInstance().getAttribute(face) //所有属性
//            val lmkDetail = DLmkDetectApi.getInstance().detectDenseLmk(
//                facePPImage,
//                face.points,
//                face.rect,
//                DLmkDetectApi.FaceDetailType.MGF_FACE_DETAIL_HAIRLINE.toInt()
//            )
//            val score = FaceDetectApi.getInstance().faceCompare(face, face) //人脸比对
                FaceDetectApi.getInstance().getExtractFeature(face) //抽取特征值
                Log.e("aaaaaaaaaaaaaaaaaaaaaaaaaa", "|1111111111");
                listener.onFace(face)
                return@execute
            }
        }
    }

    fun release() {
        FaceDetectApi.getInstance().releaseFaceDetect() //释放人脸检测
        DLmkDetectApi.getInstance().releaseDlmDetect() //释放稠密点检测
        FaceppApi.getInstance().ReleaseHandle();//释放facepp sdk
    }

    interface FacePlusListener {
        fun onFace(face: FaceDetectApi.Face?)
    }
}