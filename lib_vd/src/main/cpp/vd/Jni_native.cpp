#include <jni.h>
#include "VdPlayer.h"
#include "VdCameraRender.h"
#include "VdRecord.h"

extern "C" {
#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libavfilter/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>
}
extern "C" {
//============Player
void native_vdPlayer_onInfoPrint(JNIEnv *env, jobject *obj) {
    LOGCATE("libavcodec %s", AV_STRINGIFY(LIBAVCODEC_VERSION));
    LOGCATE("libavformat %s", AV_STRINGIFY(LIBAVFORMAT_VERSION));
    LOGCATE("libavutil %s", AV_STRINGIFY(LIBAVUTIL_VERSION));
    LOGCATE("libavfilter %s", AV_STRINGIFY(LIBAVFILTER_VERSION));
    LOGCATE("libswresample %s", AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    LOGCATE("libswscale %s", AV_STRINGIFY(LIBSWSCALE_VERSION));
    LOGCATE("avcodec_configure %s", avcodec_configuration());
    LOGCATE("avcodec_license %s", avcodec_license());
}

void native_vdPlayer_onSource(JNIEnv *env, jobject *obj, jstring url) {
    char *p = (char *) env->GetStringUTFChars(url, 0);
    VdPlayer::instance()->onSource(p);
}

void native_vdPlayer_onSeekTo(JNIEnv *env, jobject *obj, jint percent) {
    VdPlayer::instance()->onSeekTo(percent);
}

void native_vdPlayer_onPlay(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onPlay();
}

void native_vdPlayer_onPause(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onPause();
}

void native_vdPlayer_onStop(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onStop();
}

void native_vdPlayer_onRelease(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onRelease();
}

void native_vdPlayer_onSurfaceCreated(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onSurfaceCreated();
}

void native_vdPlayer_onSurfaceChanged(JNIEnv *env, jobject *obj, jint width, jint height) {
    VdPlayer::instance()->onSurfaceChanged(width, height);
}

void native_vdPlayer_onDrawFrame(JNIEnv *env, jobject *obj) {
    VdPlayer::instance()->onDrawFrame();
}
//============CameraRender
void
native_vdCameraRender_onFace(JNIEnv *env, jobject *obj, jstring string1, jstring string2,
                             jstring string3, jstring string4, jstring string5, jint faceI) {
    if (faceI == -1) {
        VdCameraRender::instance()->onFace(nullptr, nullptr, nullptr, nullptr, nullptr, -1);
    } else if (faceI == 1) {
        char *face = (char *) env->GetStringUTFChars(string1, 0);
        char *eyes = (char *) env->GetStringUTFChars(string2, 0);
        char *nose = (char *) env->GetStringUTFChars(string3, 0);
        char *mouth = (char *) env->GetStringUTFChars(string4, 0);
        VdCameraRender::instance()->onFace(face, eyes, nose, mouth, nullptr, 1);
    } else if (faceI == 2) {
        char *face = (char *) env->GetStringUTFChars(string1, 0);
        char *eyes = (char *) env->GetStringUTFChars(string2, 0);
        char *nose = (char *) env->GetStringUTFChars(string3, 0);
        char *mouth = (char *) env->GetStringUTFChars(string4, 0);
        char *alignment = (char *) env->GetStringUTFChars(string5, 0);
        VdCameraRender::instance()->onFace(face, eyes, nose, mouth, alignment, 2);
    } else if (faceI == 3) {
        VdCameraRender::instance()->onFace(nullptr, nullptr, nullptr, nullptr, nullptr, 3);
    } else if (faceI == 4) {
        char *face = (char *) env->GetStringUTFChars(string1, 0);
        char *alignment = (char *) env->GetStringUTFChars(string5, 0);
        VdCameraRender::instance()->onFace(face, nullptr, nullptr, nullptr, alignment, 4);
    }
}

void native_vdCameraRender_onBuffer(JNIEnv *env, jobject *obj, jint format, jint width, jint height,
                                    jbyteArray data) {
    int length = env->GetArrayLength(data);
    uint8_t *buffer = new uint8_t[length];
    env->GetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    VdCameraRender::instance()->onBuffer(format, width, height, buffer);
}

jbyteArray native_vdCameraRender_onBufferCapture(JNIEnv *env, jobject obj) {
    uint8_t *buffer = VdCameraRender::instance()->onBuffer();
    int length = VdCameraRender::instance()->onBufferSize();
    if (buffer == nullptr) return nullptr;
    if (length == 0) return nullptr;
    jbyteArray data = env->NewByteArray(length);
    env->SetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    return data;
}

void native_vdCameraRender_onRotate(JNIEnv *env, jobject *obj, jfloat rot, jboolean modelRot) {
    VdCameraRender::instance()->onRotate(rot, modelRot);
}

void native_vdCameraRender_onSurfaceCreated(JNIEnv *env, jobject *obj) {
    VdCameraRender::instance()->onSurfaceCreated();
}

void native_vdCameraRender_onSurfaceChanged(JNIEnv *env, jobject *obj, jint width, jint height) {
    VdCameraRender::instance()->onSurfaceChanged(width, height);
}

void native_vdCameraRender_onDrawFrame(JNIEnv *env, jobject *obj) {
    VdCameraRender::instance()->onDrawFrame();
}

void native_vdCameraRender_onRelease(JNIEnv *env, jobject *obj) {
    VdCameraRender::instance()->onRelease();
}

//============Record
void
native_vdRecord_onSource(JNIEnv *env, jobject *obj, jstring urlOut, jint w, jint h, jlong vBRate,
                         jint fps) {
    char *p = (char *) env->GetStringUTFChars(urlOut, 0);
    VdRecord::instance()->onConfig(p, w, h, vBRate, fps);
}

void native_vdRecord_onStart(JNIEnv *env, jobject *obj) {
    VdRecord::instance()->onStart();
}

void native_vdRecord_onStop(JNIEnv *env, jobject *obj) {
    VdRecord::instance()->onStop();
}

void native_vdRecord_onBufferVideo(JNIEnv *env, jobject *obj, jint format, jint w, jint h,
                                   jbyteArray data) {
    int length = env->GetArrayLength(data);
    uint8_t *buffer = new uint8_t[length];
    env->GetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    VdRecord::instance()->onBufferVideo(format, w, h, buffer);
}

void native_vdRecord_onBufferAudio(JNIEnv *env, jobject *obj, jbyteArray data) {
    int length = env->GetArrayLength(data);
    uint8_t *buffer = new uint8_t[length];
    env->GetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    VdRecord::instance()->onBufferAudio(length, buffer);
}

const char *JNI_Class_Player[] = {
        "lin/abcdq/vd/player/VdPlayer"
};
const char *JNI_Class_Camera[] = {
        "lin/abcdq/vd/camera/VdCamera"
};
const char *JNI_Class_Record[] = {
        "lin/abcdq/vd/record/VdRecord"
};
JNINativeMethod JNI_Methods_Player[] = {
        //Player
        {"native_vdPlayer_onInfoPrint",      "()V",                   (void *) native_vdPlayer_onInfoPrint},
        {"native_vdPlayer_onSource",         "(Ljava/lang/String;)V", (void *) native_vdPlayer_onSource},
        {"native_vdPlayer_onSeekTo",         "(I)V",                  (void *) native_vdPlayer_onSeekTo},
        {"native_vdPlayer_onPlay",           "()V",                   (void *) native_vdPlayer_onPlay},
        {"native_vdPlayer_onPause",          "()V",                   (void *) native_vdPlayer_onPause},
        {"native_vdPlayer_onStop",           "()V",                   (void *) native_vdPlayer_onStop},
        {"native_vdPlayer_onRelease",        "()V",                   (void *) native_vdPlayer_onRelease},
        {"native_vdPlayer_onSurfaceCreated", "()V",                   (void *) native_vdPlayer_onSurfaceCreated},
        {"native_vdPlayer_onSurfaceChanged", "(II)V",                 (void *) native_vdPlayer_onSurfaceChanged},
        {"native_vdPlayer_onDrawFrame",      "()V",                   (void *) native_vdPlayer_onDrawFrame},
};
JNINativeMethod JNI_Methods_Camera[] = {
        {"native_vdCameraRender_onFace",           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V", (void *) native_vdCameraRender_onFace},
        {"native_vdCameraRender_onBuffer",         "(III[B)V",                                                                                       (void *) native_vdCameraRender_onBuffer},
        {"native_vdCameraRender_onBufferCapture",  "()[B",                                                                                           (void *) native_vdCameraRender_onBufferCapture},
        {"native_vdCameraRender_onRotate",         "(FZ)V",                                                                                          (void *) native_vdCameraRender_onRotate},
        {"native_vdCameraRender_onSurfaceCreated", "()V",                                                                                            (void *) native_vdCameraRender_onSurfaceCreated},
        {"native_vdCameraRender_onSurfaceChanged", "(II)V",                                                                                          (void *) native_vdCameraRender_onSurfaceChanged},
        {"native_vdCameraRender_onDrawFrame",      "()V",                                                                                            (void *) native_vdCameraRender_onDrawFrame},
        {"native_vdCameraRender_onRelease",        "()V",                                                                                            (void *) native_vdCameraRender_onRelease}
};
JNINativeMethod JNI_Methods_Record[] = {
        {"native_vdRecord_onSource",      "(Ljava/lang/String;IIJI)V", (void *) native_vdRecord_onSource},
        {"native_vdRecord_onStart",       "()V",                       (void *) native_vdRecord_onStart},
        {"native_vdRecord_onStop",        "()V",                       (void *) native_vdRecord_onStop},
        {"native_vdRecord_onBufferVideo", "(III[B)V",                  (void *) native_vdRecord_onBufferVideo},
        {"native_vdRecord_onBufferAudio", "([B)V",                     (void *) native_vdRecord_onBufferAudio},
};
#define JNI_LENGTH(n) (sizeof(n) / sizeof(n[0]))

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass(JNI_Class_Player[0]);
    if (env->RegisterNatives(clazz, JNI_Methods_Player, JNI_LENGTH(JNI_Methods_Player)) != JNI_OK) {
        return JNI_ERR;
    }
    clazz = env->FindClass(JNI_Class_Camera[0]);
    if (env->RegisterNatives(clazz, JNI_Methods_Camera, JNI_LENGTH(JNI_Methods_Camera)) != JNI_OK) {
        return JNI_ERR;
    }
    clazz = env->FindClass(JNI_Class_Record[0]);
    if (env->RegisterNatives(clazz, JNI_Methods_Record, JNI_LENGTH(JNI_Methods_Record)) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

};