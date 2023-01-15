#pragma once
#include "FunctionT.h"
#include "JsExternalObjectBase.h"
#include "JsType.h"
#include <string>

class ChakraBackend {
public:
    static void *Undefined();
    static void *Null();
    static void *ExternalObject(JsExternalObjectBase *data);
    static void *Array(uint32_t n);
    static void *GlobalObject();
    static void *Bool(bool arg);
    static void *String(const std::string &arg);
    static void *Int(int arg);
    static void *Double(double arg);
    static void *Function(const FunctionT &arg);
    static void *NamedFunction(const char *name, const FunctionT &arg);
    static void *Uint8Array(uint32_t length);
    static void *ArrayBuffer(uint32_t length);
    static void *GetTypedArrayData(void *value);
    static uint32_t GetTypedArrayBufferLength(void *value);
    static void *GetArrayBufferData(void *value);
    static uint32_t GetArrayBufferLength(void *value);
    static void *ConvertValueToString(void *value);
    static std::string GetString(void *value);
    static bool GetBool(void *value);
    static int GetInt(void *value);
    static double GetDouble(void *value);
    static JsType GetType(void *value);
    static JsExternalObjectBase *GetExternalData(void *value);
    static void SetProperty(void *value, void* key, void *newValue);
    static void DefineProperty(void *value, void* key, void* descriptor);
    static void *GetProperty(void *value, void *key);
    static void *Call(void *value, void** arguments, uint32_t argumentCount, bool isConstructor);
    static void AddRef(void *value);
    static void Release(void *value);
};
