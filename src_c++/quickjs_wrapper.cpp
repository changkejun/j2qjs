#define CONFIG_BIGNUM y
//
// Created by yonglan.whl on 2021/7/14.
// Rewritten by ChangKejun on 2023/09/22
//
#include "quickjs_wrapper.h"
#include "cutils.h"
#include "quickjs-libc.h"
#include <cstring>
#include <cmath>

#define MAX_SAFE_INTEGER (((int64_t)1 << 53) - 1)

// util
static string getJavaName(JNIEnv* env, jobject javaClass) {
    auto classType = env->GetObjectClass(javaClass);
    const auto method = env->GetMethodID(classType, "getName", "()Ljava/lang/String;");
    auto javaString = (jstring)(env->CallObjectMethod(javaClass, method));
    const auto s = env->GetStringUTFChars(javaString, nullptr);

    std::string str(s);
    env->ReleaseStringUTFChars(javaString, s);
    env->DeleteLocalRef(javaString);
    env->DeleteLocalRef(classType);
    return str;
}

static void tryToTriggerOnError(JSContext *ctx, JSValueConst *error) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue onerror = JS_GetPropertyStr(ctx, global, "onError");
    if (JS_IsNull(onerror)) {
        // may be lowercase
        onerror = JS_GetPropertyStr(ctx, global, "onerror");
    }

    if (JS_IsNull(onerror)) {
        // do nothing
        return;
    }

    JS_Call(ctx, onerror, global, 1, error);
    JS_FreeValue(ctx, onerror);
    JS_FreeValue(ctx, global);
}

static string getJSErrorStr(JSContext *ctx, JSValueConst error) {
    JSValue val;
    bool is_error;
    is_error = JS_IsError(ctx, error);
    string jsException;
    if (is_error) {
        tryToTriggerOnError(ctx, &error);

        JSValue message = JS_GetPropertyStr(ctx, error, "message");
        const char *msg_str = JS_ToCString(ctx, message);
        jsException += msg_str;
        JS_FreeCString(ctx, msg_str);
        JS_FreeValue(ctx, message);

        val = JS_GetPropertyStr(ctx, error, "stack");
        if (!JS_IsUndefined(val)) {
            jsException += "\n";

            const char *stack_str = JS_ToCString(ctx, val);
            jsException += stack_str;
            JS_FreeCString(ctx, stack_str);
        }
        JS_FreeValue(ctx, val);
    } else {
        const char *error_str = JS_ToCString(ctx, error);
        jsException += error_str;
        JS_FreeCString(ctx, error_str);
    }
    return jsException;
}

static string getJSErrorStr(JSContext *ctx) {
    JSValue error = JS_GetException(ctx);
    string error_str = getJSErrorStr(ctx, error);
    JS_FreeValue(ctx, error);
    return error_str;
}

static void throwJavaException(JNIEnv *env, const char *exceptionClass, const char *fmt, ...) {
    char msg[512];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    jclass e = env->FindClass(exceptionClass);
    env->ThrowNew(e, msg);
    env->DeleteLocalRef(e);
}

static void throwJSException(JNIEnv *env, const char* msg) {
    if (env->ExceptionCheck()) {
        return;
    }

    jclass e = env->FindClass("efw/script/j2qjs/wrapper/QuickJSException");
    jmethodID init = env->GetMethodID(e, "<init>", "(Ljava/lang/String;Z)V");
    jstring ret = env->NewStringUTF(msg);
    auto t = (jthrowable)env->NewObject(e, init, ret, JNI_TRUE);
    env->Throw(t);
    env->DeleteLocalRef(e);
}

static void throwJSException(JNIEnv *env, JSContext *ctx) {
    string error = getJSErrorStr(ctx);
    throwJSException(env, error.c_str());
}

// js function callback
static JSClassID js_func_callback_class_id;

static void jsFuncCallbackFinalizer(JSRuntime *rt, JSValue val) {
    auto wrapper = reinterpret_cast<const QuickJSWrapper*>(JS_GetRuntimeOpaque(rt));
    if (wrapper) {
        int *callbackId = (int *)(JS_GetOpaque2(wrapper->context, val, js_func_callback_class_id));
        wrapper->removeCallFunction(*callbackId);
        delete callbackId;
    }
}

static JSClassDef js_func_callback_class = {
        "JSFuncCallback",
        .finalizer = jsFuncCallbackFinalizer,
};

static JSValue jsFnCallback(JSContext *ctx,
                            JSValueConst this_obj,
                            int argc, JSValueConst *argv,
                            int magic, JSValue *func_data) {

    int callbackId = *((int *)JS_GetOpaque2(ctx, func_data[0], js_func_callback_class_id));
    auto wrapper = reinterpret_cast<QuickJSWrapper*>(JS_GetRuntimeOpaque(JS_GetRuntime(ctx)));
    JSValue value = wrapper->jsFuncCall(callbackId, this_obj, argc, argv);
    return value;
}

static void initJSFuncCallback(JSContext *ctx) {
    // JSFuncCallback class
    JS_NewClassID(&js_func_callback_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_func_callback_class_id, &js_func_callback_class);
}

static bool throwIfUnhandledRejections(QuickJSWrapper *wrapper, JSContext *ctx) {
    string error;
    while (!wrapper->unhandledRejections.empty()) {
        JSValueConst reason = wrapper->unhandledRejections.front();
        error += getJSErrorStr(ctx, reason);
        error += "\n";
        JS_FreeValue(ctx, reason);
        wrapper->unhandledRejections.pop();
    }

    bool is_error = !error.empty();
    if (is_error) {
        error = "UnhandledPromiseRejectionException: " + error;
        throwJSException(wrapper->jniEnv, error.c_str());
    }
    return is_error;
}

static bool executePendingJobLoop(JNIEnv *env, JSRuntime *rt, JSContext *ctx) {
    if (env->ExceptionCheck()) {
        return false;
    }

    JSContext *ctx1;
    bool success = true;
    int err;
    /* execute the pending jobs */
    for(;;) {
        err = JS_ExecutePendingJob(rt, &ctx1);
        if (err <= 0) {
            if (err < 0) {
                success = false;
                string error = getJSErrorStr(ctx);
                throwJSException(env, error.c_str());
            }
            break;
        }
    }

    if (success && throwIfUnhandledRejections(reinterpret_cast<QuickJSWrapper *>(JS_GetRuntimeOpaque(rt)), ctx)) {
        success = false;
    }

    return success;
}
static int eval_buf(JSContext *ctx, const char *buf, int buf_len,
                    const char *filename, int eval_flags)
{
    JSValue val;
    int ret;

    if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(ctx, buf, buf_len, filename,
                      eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, TRUE, TRUE);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
    }
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}
/* also used to initialize the worker context */
static JSContext *JS_NewCustomContext(JSRuntime *rt)
{
    JSContext *ctx;
    ctx = JS_NewContext(rt);
    if (!ctx) return NULL;
    JS_AddIntrinsicBigFloat(ctx);
    JS_AddIntrinsicBigDecimal(ctx);
    JS_AddIntrinsicOperators(ctx);
    JS_EnableBignumExt(ctx, TRUE);
    /* system modules */
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");
    return ctx;
}

QuickJSWrapper::QuickJSWrapper(JNIEnv *env, jobject thiz, JSRuntime *rt) {
    jniEnv = env;
    runtime = rt;
    jniThiz = jniEnv->NewGlobalRef(thiz);
    /* loader for ES6 modules */
	js_std_set_worker_new_context_func(JS_NewCustomContext);
	js_std_init_handlers(rt);
    context = JS_NewCustomContext(runtime);
    JS_SetModuleLoaderFunc(runtime, NULL, js_module_loader, NULL);
    JS_SetHostPromiseRejectionTracker(runtime, js_std_promise_rejection_tracker, NULL);
    js_std_add_helpers(context, 0, NULL);//same methods as the mozilla JS shell
	
    const char *str = "import * as std from 'std';\n"
        "import * as os from 'os';\n"
        "globalThis.std = std;\n"
        "globalThis.os = os;\n";
    eval_buf(context, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
	
    JS_SetRuntimeOpaque(runtime, this);
    initJSFuncCallback(context);

    objectClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/Object")));
    booleanClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/Boolean")));
    integerClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/Integer")));
    longClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/Long")));
    doubleClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/Double")));
    stringClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("java/lang/String")));
    jsObjectClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("efw/script/j2qjs/wrapper/JSObject")));
    jsArrayClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("efw/script/j2qjs/wrapper/JSArray")));
    jsFunctionClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("efw/script/j2qjs/wrapper/JSFunction")));
    jsCallFunctionClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("efw/script/j2qjs/wrapper/JSCallFunction")));
    quickjsContextClass = (jclass)(jniEnv->NewGlobalRef(jniEnv->FindClass("efw/script/j2qjs/wrapper/QuickJSWrapper")));

    booleanValueOf = jniEnv->GetStaticMethodID(booleanClass, "valueOf", "(Z)Ljava/lang/Boolean;");
    integerValueOf = jniEnv->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
    longValueOf = jniEnv->GetStaticMethodID(longClass, "valueOf", "(J)Ljava/lang/Long;");
    doubleValueOf = jniEnv->GetStaticMethodID(doubleClass, "valueOf", "(D)Ljava/lang/Double;");

    booleanGetValue = jniEnv->GetMethodID(booleanClass, "booleanValue", "()Z");
    integerGetValue = jniEnv->GetMethodID(integerClass, "intValue", "()I");
    longGetValue = jniEnv->GetMethodID(longClass, "longValue", "()J");
    doubleGetValue = jniEnv->GetMethodID(doubleClass, "doubleValue", "()D");
    jsObjectGetValue = jniEnv->GetMethodID(jsObjectClass, "getPointer", "()J");

    jsObjectInit = jniEnv->GetMethodID(jsObjectClass, "<init>", "(Lefw/script/j2qjs/wrapper/QuickJSWrapper;J)V");
    jsArrayInit = jniEnv->GetMethodID(jsArrayClass, "<init>", "(Lefw/script/j2qjs/wrapper/QuickJSWrapper;J)V");
    jsFunctionInit = jniEnv->GetMethodID(jsFunctionClass, "<init>","(Lefw/script/j2qjs/wrapper/QuickJSWrapper;JJ)V");

    callFunctionBackM = jniEnv->GetMethodID(quickjsContextClass, "callFunctionBack", "(I[Ljava/lang/Object;)Ljava/lang/Object;");
    removeCallFunctionM = jniEnv->GetMethodID(quickjsContextClass, "removeCallFunction", "(I)V");
    callFunctionHashCodeM = jniEnv->GetMethodID(objectClass, "hashCode", "()I");
}

QuickJSWrapper::~QuickJSWrapper() {
    map<jlong, JSValue>::iterator i;
    for (i = values.begin(); i != values.end(); ++i) {
        JSValue item = i->second;
        JS_FreeValue(context, item);
    }
    values.clear();

    JS_FreeContext(context);
    JS_FreeRuntime(runtime);

    jniEnv->DeleteGlobalRef(jniThiz);
    jniEnv->DeleteGlobalRef(objectClass);
    jniEnv->DeleteGlobalRef(doubleClass);
    jniEnv->DeleteGlobalRef(integerClass);
    jniEnv->DeleteGlobalRef(longClass);
    jniEnv->DeleteGlobalRef(booleanClass);
    jniEnv->DeleteGlobalRef(stringClass);
    jniEnv->DeleteGlobalRef(jsObjectClass);
    jniEnv->DeleteGlobalRef(jsArrayClass);
    jniEnv->DeleteGlobalRef(jsFunctionClass);
    jniEnv->DeleteGlobalRef(jsCallFunctionClass);
    jniEnv->DeleteGlobalRef(quickjsContextClass);
}

jobject QuickJSWrapper::toJavaObject(JNIEnv *env, jobject thiz, JSValueConst& this_obj, JSValueConst& value, bool non_js_callback){
    jobject result;
    switch (JS_VALUE_GET_NORM_TAG(value)) {
        case JS_TAG_EXCEPTION: {
            result = nullptr;
            break;
        }

        case JS_TAG_STRING: {
            const char* string = JS_ToCString(context, value);
            result = env->NewStringUTF(string);
            JS_FreeCString(context, string);
            if (non_js_callback) {
                // JSString 类型的 JSValue 需要手动释放掉，不然会泄漏
                JS_FreeValue(context, value);
            }
            break;
        }

        case JS_TAG_BOOL: {
            jvalue v;
            v.z = static_cast<jboolean>(JS_VALUE_GET_BOOL(value));
            result = env->CallStaticObjectMethodA(booleanClass, booleanValueOf, &v);
            break;
        }

        case JS_TAG_INT: {
            jvalue v;
            v.j = static_cast<jint>(JS_VALUE_GET_INT(value));
            result = env->CallStaticObjectMethodA(integerClass, integerValueOf, &v);
            break;
        }

        case JS_TAG_BIG_INT: {
            int64_t e;
            JS_ToBigInt64(context, &e, value);
            jvalue v;
            v.j = e;
            result = env->CallStaticObjectMethodA(longClass, longValueOf, &v);
            break;
        }

        case JS_TAG_FLOAT64: {
            jvalue v;
            double d = JS_VALUE_GET_FLOAT64(value);
            bool isInteger = floor(d) == d;
            if (isInteger) {
                v.j = static_cast<jlong>(d);
                result = env->CallStaticObjectMethodA(longClass, longValueOf, &v);
            } else {
                v.d = static_cast<jdouble>(d);
                result = env->CallStaticObjectMethodA(doubleClass, doubleValueOf, &v);
            }
            break;
        }

        case JS_TAG_OBJECT: {
            auto value_ptr = reinterpret_cast<jlong>(JS_VALUE_GET_PTR(value));
            if (JS_IsFunction(context, value)) {
                auto obj_ptr = reinterpret_cast<jlong>(JS_VALUE_GET_PTR(this_obj));
                result = env->NewObject(jsFunctionClass, jsFunctionInit, thiz, obj_ptr, value_ptr);
            } else if (JS_IsArray(context, value)) {
                result = env->NewObject(jsArrayClass, jsArrayInit, thiz, value_ptr);
            } else {
                result = env->NewObject(jsObjectClass, jsObjectInit, thiz, value_ptr);
            }

            if (non_js_callback) {
                // 这里对 JSObject 的引用计数做了处理：
                // 1. 判断是否有该对象，如果没有就保存到 values 里
                // 2. 如果已经有该对象，则对其进行计数减一
                // 3. 这样，可以保证该对象引用计数一直只会加一次
                // 4. 如果没有主动释放，会在 destroy 的时候进行减一
                if (values.count(value_ptr) == 0) {
                    values.insert(pair<jlong, JSValue>(value_ptr, value));
                } else{
                    JS_FreeValue(context, value);
                }
            }

            break;
        }

        default:
            result = nullptr;
            break;
    }

    return result;
}

jobject QuickJSWrapper::evaluate(JNIEnv *env, jobject thiz, jstring script, jstring file_name) {
    const char *c_script = env->GetStringUTFChars(script, JNI_FALSE);
    const char *c_file_name = env->GetStringUTFChars(file_name, JNI_FALSE);

    JSValue result = JS_Eval(context, c_script, strlen(c_script), c_file_name, JS_EVAL_TYPE_GLOBAL);
    env->ReleaseStringUTFChars(script, c_script);
    env->ReleaseStringUTFChars(file_name, c_file_name);
    if (JS_IsException(result)) {
        throwJSException(env, context);
        return nullptr;
    }

    if (!executePendingJobLoop(env, runtime, context)) {
        JS_FreeValue(context, result);
        return nullptr;
    }

    JSValue global = JS_GetGlobalObject(context);
    jobject jsObj = toJavaObject(env, thiz, global, result);
    JS_FreeValue(context, global);

    return jsObj;
}

jobject QuickJSWrapper::getGlobalObject(JNIEnv *env, jobject thiz) const {
    JSValue value = JS_GetGlobalObject(context);

    auto value_ptr = reinterpret_cast<jlong>(JS_VALUE_GET_PTR(value));
    jobject result = env->NewObject(jsObjectClass, jsObjectInit, thiz, value_ptr);

    JS_FreeValue(context, value);
    return result;
}

jobject QuickJSWrapper::getProperty(JNIEnv *env, jobject thiz, jlong value, jstring name) {
    JSValue jsObject = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(value));

    const char *propsName = env->GetStringUTFChars(name, JNI_FALSE);
    JSValue propsValue = JS_GetPropertyStr(context, jsObject, propsName);
    env->ReleaseStringUTFChars(name, propsName);
    if (JS_IsException(propsValue)) {
        throwJSException(env, context);
        return nullptr;
    }

    return toJavaObject(env, thiz, jsObject, propsValue);
}

jobject QuickJSWrapper::call(JNIEnv *env, jobject thiz, jlong func, jlong this_obj,
                             jobjectArray args) {
    int argc = env->GetArrayLength(args);
    vector<JSValue> arguments;
    vector<JSValue> freeArguments;
    for (int numArgs = 0; numArgs < argc && !env->ExceptionCheck(); numArgs++) {
        jobject arg = env->GetObjectArrayElement(args, numArgs);
        auto jsArg = toJSValue(env, thiz, arg);
        if (JS_IsException(jsArg)) {
            return nullptr;
        }

        // 基础类型(例如 string )和 Java callback 类型需要使用完 free.
        if (env->IsInstanceOf(arg, stringClass) || env->IsInstanceOf(arg, doubleClass) ||
            env->IsInstanceOf(arg, integerClass) || env->IsInstanceOf(arg, longClass) ||
            env->IsInstanceOf(arg, booleanClass) || env->IsInstanceOf(arg, jsCallFunctionClass)) {
            freeArguments.push_back(jsArg);
        }

        env->DeleteLocalRef(arg);

        arguments.push_back(jsArg);
    }

    JSValue jsObj;
    if (this_obj == 0) {
        jsObj = JS_UNDEFINED;
    } else {
        jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(this_obj));
    }

    JSValue jsFunc = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(func));

    JSValue ret = JS_Call(context, jsFunc, jsObj, arguments.size(), arguments.data());
    if (JS_IsException(ret)) {
        throwJSException(env, context);
        return nullptr;
    }

    for (JSValue argument : freeArguments) {
        JS_FreeValue(context, argument);
    }

    // release vector by swap.
    vector<JSValue>().swap(arguments);
    vector<JSValue>().swap(freeArguments);

    if (!executePendingJobLoop(env, runtime, context)) {
        JS_FreeValue(context, ret);
        return nullptr;
    }

    return toJavaObject(env, thiz, jsObj, ret);
}

jstring QuickJSWrapper::jsonStringify(JNIEnv *env, jlong value) const {
    JSValue obj = JS_JSONStringify(context, JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(value)), JS_UNDEFINED, JS_UNDEFINED);
    if (JS_IsException(obj)){
        throwJSException(env, context);
        return nullptr;
    }

    auto result = JS_ToCString(context, obj);
    JS_FreeValue(context, obj);
    jstring string =env->NewStringUTF(result);
    JS_FreeCString(context, result);
    return string;
}

jint QuickJSWrapper::length(JNIEnv *env, jlong value) const {
    JSValue jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(value));

    JSValue length = JS_GetPropertyStr(context, jsObj, "length");
    if (JS_IsException(length)) {
        throwJSException(env, context);
        return -1;
    }

    JS_FreeValue(context, length);

    return JS_VALUE_GET_INT(length);
}

jobject QuickJSWrapper::get(JNIEnv *env, jobject thiz, jlong value, jint index) {
    JSValue jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(value));
    JSValue child = JS_GetPropertyUint32(context, jsObj, index);

    return toJavaObject(env, thiz, jsObj, child);
}

void QuickJSWrapper::set(JNIEnv *env, jobject thiz, jlong this_obj, jobject value, jint index) {
    JSValue jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(this_obj));
    JSValue child = toJSValue(env, thiz, value);
    if (JS_IsString(child)) {
        // JSString 类型不需要 JS_DupValue
        JS_SetPropertyUint32(context, jsObj, index, child);
    } else {
        JS_SetPropertyUint32(context, jsObj, index, JS_DupValue(context, child));
    }

}

void
QuickJSWrapper::setProperty(JNIEnv *env, jobject thiz, jlong this_obj, jstring name, jobject value) const {
    const char* propName = env->GetStringUTFChars(name, JNI_FALSE);
    JSValue propValue = toJSValue(env, thiz, value);
    if(env->IsInstanceOf(value, jsObjectClass)) {
        // 这里需要手动增加引用计数，不然 QuickJS 垃圾回收会报 assertion "p->ref_count > 0" 的错误。
        JS_DupValue(context, propValue);
    } else if (env->IsInstanceOf(value, jsCallFunctionClass)) {
        // 通过 JS_NewCFunctionData 创建的 fn 对象的 name 属性值被定义为 Empty 了，
        // 这里需要额外定义下，不然 js 层拿到的 fn.name 的值为空.
        JSAtom name_atom = JS_NewAtom(context, propName);
        JSAtom name_atom_key = JS_NewAtom(context, "name");
        JS_DefinePropertyValue(context, propValue, name_atom_key,
                               JS_AtomToString(context, name_atom), JS_PROP_CONFIGURABLE);
        JS_FreeAtom(context, name_atom);
        JS_FreeAtom(context, name_atom_key);
    }

    JSValue jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(this_obj));
    JS_SetPropertyStr(context, jsObj, propName, propValue);

    env->ReleaseStringUTFChars(name, propName);
}

JSValue QuickJSWrapper::jsFuncCall(int callback_id, JSValueConst this_val, int argc, JSValueConst *argv){
    jobjectArray javaArgs = jniEnv->NewObjectArray((jsize)argc, objectClass, nullptr);

    for (int i = 0; i < argc; i++) {
        auto java_arg = toJavaObject(jniEnv, jniThiz, this_val, argv[i], false);
        jniEnv->SetObjectArrayElement(javaArgs, (jsize)i, java_arg);
        jniEnv->DeleteLocalRef(java_arg);
    }

    auto result = jniEnv->CallObjectMethod(jniThiz, callFunctionBackM, callback_id, javaArgs);

    jniEnv->DeleteLocalRef(javaArgs);

    JSValue jsValue = toJSValue(jniEnv, jniThiz, result);

    // JS 对象作为方法返回值，需要引用计数加1，不然会被释放掉
    if (JS_IsObject(jsValue) && !jniEnv->IsInstanceOf(result, jsCallFunctionClass)) {
        JS_DupValue(context, jsValue);

        // todo 这里有一个引用计数优化点
        // 1. 通过 Java 层创建的 JSObject 会被计数加1，上面的 JS_DupValue 会加1，
        // 2. 也就是会有两次引用，在函数执行完会减1，因为还有一次引用不会被释放。
        // 3. 优化方式一：注释掉上面的 JS_DupValue 代码，放开下面的注释代码。
//        map<jlong, JSValue>::iterator exist;
//        exist = values.find(reinterpret_cast<jlong>(JS_VALUE_GET_PTR(jsValue)));
//        if (exist != values.end()) {
//            values.erase(exist);
//        }
    }

    jniEnv->DeleteLocalRef(result);
    return jsValue;
}

void QuickJSWrapper::removeCallFunction(int callback_id) const {
    jniEnv->CallVoidMethod(jniThiz, removeCallFunctionM, callback_id);
}

JSValue QuickJSWrapper::toJSValue(JNIEnv *env, jobject thiz, jobject value) const {
    if (value == nullptr) {
        return JS_UNDEFINED;
    }

    JSValue result;
    if (env->IsInstanceOf(value, stringClass)) {
        const auto s = env->GetStringUTFChars((jstring)(value), JNI_FALSE);
        result = JS_NewString(context, s);
        env->ReleaseStringUTFChars((jstring)(value), s);
    } else if (env->IsInstanceOf(value, doubleClass)) {
        result = JS_NewFloat64(context, env->CallDoubleMethod(value, doubleGetValue));
    } else if (env->IsInstanceOf(value, integerClass)) {
        result = JS_NewInt32(context, env->CallIntMethod(value, integerGetValue));
    } else if(env->IsInstanceOf(value, longClass)) {
        jlong l_val = env->CallLongMethod(value, longGetValue);
        if (l_val > MAX_SAFE_INTEGER || l_val < -MAX_SAFE_INTEGER) {
            result = JS_NewBigInt64(context, l_val);
        } else {
            result = JS_NewInt64(context, l_val);
        }
    } else if (env->IsInstanceOf(value, booleanClass)) {
        result = JS_NewBool(context, env->CallBooleanMethod(value, booleanGetValue));
    } else if (env->IsInstanceOf(value, jsObjectClass)) {
        result = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(env->CallLongMethod(value, jsObjectGetValue)));
    } else if (env->IsInstanceOf(value, jsCallFunctionClass)) {
        // 这里的 obj 是用来获取 JSFuncCallback 对象的
        JSValue obj = JS_NewObjectClass(context, js_func_callback_class_id);
        result = JS_NewCFunctionData(context, jsFnCallback, 1, 0, 1, &obj);
        // JS_NewCFunctionData 有 dupValue obj，这里需要对 obj 计数减一，保持计数平衡
        JS_FreeValue(context, obj);

        int *callbackId = new int(jniEnv->CallIntMethod(value, callFunctionHashCodeM));
        JS_SetOpaque(obj, callbackId);
    } else {
        auto classType = env->GetObjectClass(value);
        const auto typeName = getJavaName(env, classType);
        env->DeleteLocalRef(classType);
        // Throw an exception for unsupported argument type.
        throwJavaException(env, "java/lang/IllegalArgumentException", "Unsupported Java type %s",
                           typeName.c_str());
        result = JS_EXCEPTION;
    }

    return result;
}

void QuickJSWrapper::freeValue(jlong value) {
    // current only free exist value in map.
    // todo refactor
    map<jlong, JSValue>::iterator exist;
    exist = values.find(value);
    if (exist != values.end()) {
        values.erase(exist);

        JSValue jsObj = JS_MKPTR(JS_TAG_OBJECT, reinterpret_cast<void *>(value));
        JS_FreeValue(context, jsObj);
    }
}

jobject QuickJSWrapper::parseJSON(JNIEnv *env, jobject thiz, jstring json) {
    const char *c_json = env->GetStringUTFChars(json, JNI_FALSE);
    auto jsonObj = JS_ParseJSON(context, c_json, strlen(c_json), "parseJSON.js");
    if (JS_IsException(jsonObj)) {
        throwJSException(env, context);
        return nullptr;
    }

    JSValue jsObj = JS_UNDEFINED;
    jobject result = toJavaObject(env, thiz, jsObj, jsonObj);
    env->ReleaseStringUTFChars(json, c_json);
    return result;
}

jbyteArray QuickJSWrapper::compile(JNIEnv *env, jstring source, jstring file_name, jboolean isModule) const {
    const auto sourceCode = env->GetStringUTFChars(source, JNI_FALSE);
    const auto fileName = env->GetStringUTFChars(file_name, JNI_FALSE);
    auto eval_flags =  JS_EVAL_FLAG_COMPILE_ONLY;
    if (isModule) {
        eval_flags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
    }
    auto compiled = JS_Eval(context, sourceCode, strlen(sourceCode), fileName, eval_flags);
    env->ReleaseStringUTFChars(source, sourceCode);
    env->ReleaseStringUTFChars(file_name, fileName);

    if (JS_IsException(compiled)) {
        throwJSException(env, context);
        return nullptr;
    }

    size_t bufferLength = 0;
    auto buffer = JS_WriteObject(context, &bufferLength, compiled, JS_WRITE_OBJ_BYTECODE | JS_WRITE_OBJ_REFERENCE);

    auto result = buffer && bufferLength > 0 ? env->NewByteArray(bufferLength) : nullptr;
    if (result) {
        env->SetByteArrayRegion(result, 0, bufferLength, reinterpret_cast<const jbyte*>(buffer));
    } else {
        throwJSException(env, context);
    }

    JS_FreeValue(context, compiled);
    js_free(context, buffer);

    return result;
}

jobject QuickJSWrapper::execute(JNIEnv *env, jobject thiz, jbyteArray byteCode) {
    const auto buffer = env->GetByteArrayElements(byteCode, nullptr);
    const auto bufferLength = env->GetArrayLength(byteCode);
    const auto flags = JS_READ_OBJ_BYTECODE | JS_READ_OBJ_REFERENCE;
    auto obj = JS_ReadObject(context, reinterpret_cast<const uint8_t*>(buffer), bufferLength, flags);
    env->ReleaseByteArrayElements(byteCode, buffer, JNI_ABORT);

    if (JS_IsException(obj)) {
        throwJSException(env, context);
        return nullptr;
    }

    if (JS_ResolveModule(context, obj)) {
        // TODO throwJsExceptionFmt(env, this, "Failed to resolve JS module");
        return nullptr;
    }

    auto val = JS_EvalFunction(context, obj);

    if (!executePendingJobLoop(env, runtime, context)) {
        JS_FreeValue(context, val);
        return nullptr;
    }

    jobject result;
    if (!JS_IsException(val)) {
        result = toJavaObject(env, thiz, obj, val);
    } else {
        result = nullptr;
        throwJSException(env, context);
    }

    return result;
}

jobject
QuickJSWrapper::evaluateModule(JNIEnv *env, jobject thiz, jstring script, jstring file_name) {
    const char *c_script = env->GetStringUTFChars(script, JNI_FALSE);
    const char *c_file_name = env->GetStringUTFChars(file_name, JNI_FALSE);

    JSValue result = JS_Eval(context, c_script, strlen(c_script), c_file_name, JS_EVAL_TYPE_MODULE);
    env->ReleaseStringUTFChars(script, c_script);
    env->ReleaseStringUTFChars(file_name, c_file_name);
    if (JS_IsException(result)) {
        throwJSException(env, context);
        return nullptr;
    }

    if (!executePendingJobLoop(env, runtime, context)) {
        JS_FreeValue(context, result);
        return nullptr;
    }

    JSValue global = JS_GetGlobalObject(context);
    jobject jsObj = toJavaObject(env, thiz, global, result);
    JS_FreeValue(context, global);
    return jsObj;
}
