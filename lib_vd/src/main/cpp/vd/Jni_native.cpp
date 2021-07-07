#include <jni.h>
#include "VdPlayer.h"
#include "VdCameraRender.h"

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
void native_vdCameraRender_onBuffer(JNIEnv *env, jobject *obj, jint format, jint width, jint height,
                                    jbyteArray data) {
    int length = env->GetArrayLength(data);
    uint8_t *buffer = new uint8_t[length];
    env->GetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    VdCameraRender::instance()->onBuffer(format, width, height, buffer);
}

jbyteArray native_vdCameraRender_OnBuffer(JNIEnv *env, jobject obj) {
    uint8_t *buffer = VdCameraRender::instance()->onBuffer();
    int length = VdCameraRender::instance()->onBufferSize();
    if (buffer == nullptr) return nullptr;
    if (length == 0) return nullptr;
    jbyteArray data = env->NewByteArray(length);
    env->SetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
    return data;
}

void native_vdCameraRender_onRotate(JNIEnv *env, jobject *obj, jfloat rotate) {
    VdCameraRender::instance()->onRotate(rotate);
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

const char *JNI_Class_Player[] = {
        "lin/abcdq/vd/player/VdPlayer"
};
const char *JNI_Class_Camera[] = {
        "lin/abcdq/vd/camera/VdCamera"
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
        {"native_vdCameraRender_onBuffer",         "(III[B)V", (void *) native_vdCameraRender_onBuffer},
        {"native_vdCameraRender_onBuffer",         "()[B",     (void *) native_vdCameraRender_onBuffer},
        {"native_vdCameraRender_onRotate",         "(F)V",      (void *) native_vdCameraRender_onRotate},
        {"native_vdCameraRender_onSurfaceCreated", "()V",      (void *) native_vdCameraRender_onSurfaceCreated},
        {"native_vdCameraRender_onSurfaceChanged", "(II)V",    (void *) native_vdCameraRender_onSurfaceChanged},
        {"native_vdCameraRender_onDrawFrame",      "()V",      (void *) native_vdCameraRender_onDrawFrame},
        {"native_vdCameraRender_onRelease",        "()V",      (void *) native_vdCameraRender_onRelease}
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
    return JNI_VERSION_1_6;
}
};