// Rewritten by ChangKejun on 2023/09/22
#include "jni.h"
#include <string>
#include "quickjs_wrapper.h"
#include <vector>

extern "C"
JNIEXPORT void JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_destroyContext(JNIEnv *env, jobject thiz,
                                                           jlong context) {
    delete reinterpret_cast<QuickJSWrapper*>(context);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_evaluate(JNIEnv *env, jobject thiz, jlong context, jstring script,
                                                     jstring file_name) {
    if (script == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "Script cannot be null");
        return nullptr;
    }

    if (file_name == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "File name cannot be null");
        return nullptr;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->evaluate(env, thiz, script, file_name);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_getGlobalObject(JNIEnv *env, jobject thiz,
                                                            jlong context) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->getGlobalObject(env, thiz);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_getProperty(JNIEnv *env, jobject thiz, jlong context, jlong value,
                                                 jstring name) {
    if (name == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "Property Name cannot be null");
        return nullptr;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->getProperty(env, thiz, value, name);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_call(JNIEnv *env, jobject thiz, jlong context,
                                                 jlong func, jlong this_obj, jobjectArray args) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->call(env, thiz, func, this_obj, args);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_stringify(JNIEnv *env, jobject thiz, jlong context,
                                               jlong value) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->jsonStringify(env, value);
}extern "C"
JNIEXPORT jint JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_length(JNIEnv *env, jobject thiz, jlong context,
                                               jlong value) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->length(env, value);
}extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_get(JNIEnv *env, jobject thiz, jlong context, jlong value,
                                         jint index) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->get(env, thiz, value, index);
}extern "C"
JNIEXPORT jlong JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_createContext(JNIEnv *env, jobject thiz, jlong runtime) {
    auto *wrapper = new(std::nothrow) QuickJSWrapper(env, thiz, reinterpret_cast<JSRuntime *>(runtime));
    if (!wrapper || !wrapper->context || !wrapper->runtime) {
        delete wrapper;
        wrapper = nullptr;
    }

    return reinterpret_cast<jlong>(wrapper);
}extern "C"
JNIEXPORT void JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_setProperty(JNIEnv *env, jobject thiz, jlong context,
                                                        jlong this_obj, jstring name,
                                                        jobject value) {
    if (name == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "Property Name cannot be null");
        return;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    wrapper->setProperty(env, thiz, this_obj, name, value);
}extern "C"
JNIEXPORT void JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_freeValue(JNIEnv *env, jobject thiz, jlong context,
                                                      jlong value) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    wrapper->freeValue(value);
}extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_parseJSON(JNIEnv *env, jobject thiz, jlong context,
                                                      jstring json) {
    if (json == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "JSON cannot be null");
        return nullptr;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->parseJSON(env, thiz, json);
}extern "C"
JNIEXPORT jbyteArray JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_compile(JNIEnv *env, jobject thiz, jlong context,
                                                    jstring source_code, jstring file_name, jboolean isModule) {
    if (source_code == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "Source code cannot be null");
        return nullptr;
    }

    if (file_name == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "File name cannot be null");
        return nullptr;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->compile(env, source_code, file_name, isModule);
}extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_execute(JNIEnv *env, jobject thiz, jlong context,
                                                    jbyteArray bytecode) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->execute(env, thiz, bytecode);
}extern "C"
JNIEXPORT jobject JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_evaluateModule(JNIEnv *env, jobject thiz, jlong context,
                                                           jstring script, jstring file_name) {
    if (script == nullptr) {                                                           
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "Script cannot be null");
        return nullptr;
    }

    if (file_name == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), "File name cannot be null");
        return nullptr;
    }

    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    return wrapper->evaluateModule(env, thiz, script, file_name);
}extern "C"
JNIEXPORT void JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_set(JNIEnv *env, jobject thiz, jlong context,
                                                jlong this_obj, jobject value, jint index) {
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(context);
    wrapper->set(env, thiz, this_obj, value, index);
}
extern "C"
JNIEXPORT void JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_runGC(JNIEnv *env, jclass thiz, jlong runtime) {
    auto *rt = reinterpret_cast<JSRuntime*>(runtime);
    JS_RunGC(rt);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_efw_script_j2qjs_wrapper_QuickJSWrapper_createRuntime(JNIEnv *env, jclass clazz) {
    auto *rt = JS_NewRuntime();
    return reinterpret_cast<jlong>(rt);
}
