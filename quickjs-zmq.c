/*
MIT License

Copyright (c) 2022 John O'Connor (sax1johno@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define _GNU_SOURCE
#include <zmq.h>
#include <stdio.h>
#include <string.h>
// #include <zhelpers.h>
#include "../quickjs/quickjs-libc.h"
#include "../quickjs/quickjs.h"

// Borrowed from quickjs-ffi
#if UINTPTR_MAX == UINT32_MAX
#define JS_TO_UINTPTR_T(ctx, pres, val) JS_ToInt32(ctx, (int32_t *)(pres), val)
#define JS_NEW_UINTPTR_T(ctx, val) JS_NewInt32(ctx, (int32_t)(val))
#define C_MACRO_UINTPTR_T_DEF(x) JS_PROP_INT32_DEF(#x, (int32_t)(x), JS_PROP_CONFIGURABLE)
// 不能#define C_MACRO_INTPTR_DEF(x) C_MACRO_INT_DEF(x)否则#会展开为x所定义的内容而非x本身
#define C_VAR_ADDRESS_DEF(x) JS_PROP_INT32_DEF(#x, (int32_t)(&x), JS_PROP_CONFIGURABLE)
#define ffi_type_intptr_t ffi_type_sint32
#define ffi_type_uintptr_t ffi_type_uint32
#elif UINTPTR_MAX == UINT64_MAX
#define JS_TO_UINTPTR_T(ctx, pres, val) JS_ToInt64(ctx, (int64_t *)(pres), val)
#define JS_NEW_UINTPTR_T(ctx, val) JS_NewInt64(ctx, (int64_t)(val))
#define C_MACRO_UINTPTR_T_DEF(x) JS_PROP_INT64_DEF(#x, (int64_t)(x), JS_PROP_CONFIGURABLE)
#define C_VAR_ADDRESS_DEF(x) JS_PROP_INT64_DEF(#x, (int64_t)(&x), JS_PROP_CONFIGURABLE)
#define ffi_type_intptr_t ffi_type_sint64
#define ffi_type_uintptr_t ffi_type_uint64
#else
#error "'uintptr_t' neither 32bit nor 64 bit, I don't know how to handle it."
#endif

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSValue js_zmq_version(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    JSValue returnObject = JS_NewObject(ctx);
    JS_DefinePropertyValueStr(ctx, returnObject, "major", JS_NewInt32(ctx, major), JS_PROP_C_W_E);
    JS_DefinePropertyValueStr(ctx, returnObject, "minor", JS_NewInt32(ctx, minor), JS_PROP_C_W_E);
    JS_DefinePropertyValueStr(ctx, returnObject, "patch", JS_NewInt32(ctx, patch), JS_PROP_C_W_E);
    return returnObject;
}

static JSValue js_zmq_new_context(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    void* context = zmq_ctx_new();
    return JS_NEW_UINTPTR_T(ctx, context);
}

static JSValue js_zmq_destroy_context(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    void* zmqContextPtr;
    JS_TO_UINTPTR_T(ctx, zmqContextPtr, argv[0]);
    zmq_ctx_destroy(zmqContextPtr);
    return JS_UNDEFINED;
}

static JSValue js_zmq_get_context_option(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    void* zmqContextPtr;
    JS_TO_UINTPTR_T(ctx, zmqContextPtr, argv[0]);
    int32_t optionName;
    JS_ToInt32(ctx, &optionName, argv[1]);
    int returnValue = zmq_ctx_get(zmqContextPtr, optionName);
    return JS_NewInt32(ctx, returnValue);
}

static JSValue js_zmq_set_context_option(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    void* zmqContextPtr;
    JS_TO_UINTPTR_T(ctx, zmqContextPtr, argv[0]);
    int32_t optionName;
    JS_ToInt32(ctx, &optionName, argv[1]);
    int32_t optionValue;
    JS_ToInt32(ctx, &optionValue, argv[2]);
    int returnCode = zmq_ctx_set(zmqContextPtr, optionName, optionValue);
    return JS_NewInt32(ctx, returnCode);
}


static JSValue js_zmq_create_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    void* zmqContextPtr;
    JS_TO_UINTPTR_T(ctx, &zmqContextPtr, argv[0]);
    int32_t socketTypeInt;
    JS_ToInt32(ctx, &socketTypeInt, argv[1]);
    void* sock = zmq_socket(zmqContextPtr, socketTypeInt);
    return JS_NEW_UINTPTR_T(ctx, sock);
    // return JS_UNDEFINED;
}

static JSValue js_zmq_close_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    void* zmqSocketPtr;
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    zmq_close(zmqSocketPtr);
    return JS_UNDEFINED;
}


static JSValue js_zmq_bind_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    void* zmqSocketPtr;
    JSValueConst addressVal = argv[1];   
    const char *address = NULL;    
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    address = JS_ToCString(ctx, addressVal);
    int returnCode = zmq_bind(zmqSocketPtr, address);
    return JS_NewInt32(ctx, returnCode);
}

static JSValue js_zmq_connect_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    void* zmqSocketPtr;
    JSValueConst addressVal = argv[1];   
    const char *address = NULL;    
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    address = JS_ToCString(ctx, addressVal);
    int returnCode = zmq_connect(zmqSocketPtr, address);
    return JS_NewInt32(ctx, returnCode);
}

static const int RECV_BUFFER_SIZE=1024;

static JSValue js_zmq_recv_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    void* zmqSocketPtr;
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    char buffer[RECV_BUFFER_SIZE];
    int returnCode = zmq_recv(zmqSocketPtr, buffer, RECV_BUFFER_SIZE, 0);
    if (returnCode < 0) {
        const char* errorString = zmq_strerror(zmq_errno());
        return JS_ThrowInternalError(ctx, errorString);
    }
    JSValue returnValue = JS_NewString(ctx, buffer);
    memset(&buffer, 0, RECV_BUFFER_SIZE); // Clear the buffer.
    return returnValue;
}

static JSValue js_zmq_send_socket(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    void* zmqSocketPtr;
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    JSValueConst messageVal = argv[1];
    const char *message = NULL;
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    size_t messageLength;
    message = JS_ToCStringLen(ctx, &messageLength, messageVal);
    int response = zmq_send(zmqSocketPtr, message, messageLength, 0);
    JS_FreeCString(ctx, message);
    return JS_NewInt32(ctx, response);
}

static JSValue js_zmq_strerror(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    JSValueConst returnCode = argv[0];
    int errCode;
    JS_ToInt32(ctx, &errCode, returnCode);
    const char* errorString = zmq_strerror(errCode);
    return JS_NewString(ctx, errorString);
}

static JSValue js_zmq_errno(JSContext* ctx, JSValueConst this_val, int argc, JSValue* argv) {
    return JS_NewInt32(ctx, zmq_errno());
}

static JSValue js_zmq_get_socket_option(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    void* zmqSocketPtr;
    JS_TO_UINTPTR_T(ctx, &zmqSocketPtr, argv[0]);
    int32_t optionName;
    JS_ToInt32(ctx, &optionName, argv[1]);
    void* optionValue;
    JS_TO_UINTPTR_T(ctx, &optionValue, argv[2]);
    size_t optionValueLength;
    JS_TO_UINTPTR_T(ctx, &optionValueLength, argv[3]);
    int returnCode = zmq_getsockopt(zmqSocketPtr, optionName, optionValue, optionValueLength);
    return JS_NewInt32(ctx, returnCode);
}

static JSValue js_zmq_set_socket_option(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    void* zmqSocketPtr;
    JS_TO_UINTPTR_T(ctx, zmqSocketPtr, argv[0]);
    int32_t optionName;
    JS_ToInt32(ctx, &optionName, argv[1]);
    int32_t optionValue;
    JS_TO_UINTPTR_T(ctx, &optionValue, argv[2]);
    size_t optionValueLength;
    JS_TO_UINTPTR_T(ctx, &optionValueLength, argv[3]);
    int returnCode = zmq_setsockopt(zmqSocketPtr, optionName, optionValue, optionValueLength);
    return JS_NewInt32(ctx, returnCode);
}


static JSCFunctionListEntry funcs[] = {
    JS_CFUNC_DEF("version", 0, js_zmq_version),
    JS_CFUNC_DEF("createContext", 0, js_zmq_new_context),
    JS_CFUNC_DEF("destroyContext", 1, js_zmq_destroy_context),
    JS_CFUNC_DEF("getContextOption", 1, js_zmq_get_context_option),
    JS_CFUNC_DEF("setContextOption", 2, js_zmq_set_context_option),
    JS_CFUNC_DEF("createSocket", 2, js_zmq_create_socket),
    JS_CFUNC_DEF("closeSocket", 1, js_zmq_close_socket),
    JS_CFUNC_DEF("bindSocket", 2, js_zmq_bind_socket),
    JS_CFUNC_DEF("recvSocket", 1, js_zmq_recv_socket),
    JS_CFUNC_DEF("sendSocket", 2, js_zmq_send_socket),
    JS_CFUNC_DEF("connectSocket", 2, js_zmq_connect_socket),
    JS_CFUNC_DEF("strerror", 1, js_zmq_strerror),
    JS_CFUNC_DEF("errno", 0, js_zmq_errno)
};


static int init(JSContext *ctx, JSModuleDef *m) {
    // JS_NewClassID(&js_dtor_class_id);
    // JS_NewClass(JS_GetRuntime(ctx), js_dtor_class_id, &js_dtor_class);
    JS_SetModuleExportList(ctx, m, funcs, countof(funcs));
    return 0;
}

JSModuleDef *js_init_module(JSContext *ctx, const char *module_name) {
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, funcs, countof(funcs));
    return m;
}