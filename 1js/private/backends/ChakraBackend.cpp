#include "ChakraBackend.h"
#include "ChakraBackendUtils.h"

#include <ChakraCore.h>

#define JS_ENGINE_F(func) func, #func

void *ChakraBackend::Undefined() {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetUndefinedValue), &v);
    return v;
}

void *ChakraBackend::Null() {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetNullValue), &v);
    return v;
}

void *ChakraBackend::ExternalObject(JsExternalObjectBase *data) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateExternalObject), data, nullptr, &v);
    return v;
}

void *ChakraBackend::Array(uint32_t n) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateArray), n, &v);
    return v;
}

void *ChakraBackend::GlobalObject() {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetGlobalObject), &v);
    return v;
}

void *ChakraBackend::Bool(bool arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsBoolToBoolean), arg, &v);
    return v;
}

void *ChakraBackend::String(const std::string &arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateString), arg.data(), arg.size(), &v);
    return v;
}

void *ChakraBackend::Int(int arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsIntToNumber), arg, &v);
    return v;
}

void *ChakraBackend::Double(double arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsDoubleToNumber), arg, &v);
    return v;
}

// TODO: fix memory leak (new FunctionT(arg))
void *ChakraBackend::Function(const FunctionT &arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateFunction), ChakraBackendUtils::NativeFunctionImpl,
             new FunctionT(arg), &v);
    return v;
}

// TODO: fix memory leak (new FunctionT(arg))
void *ChakraBackend::NamedFunction(const char *name, const FunctionT &arg) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateNamedFunction), name,
             ChakraBackendUtils::NativeFunctionImpl, new FunctionT(arg), &v);
    return v;
}

void *ChakraBackend::Uint8Array(uint32_t length) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateTypedArray), JsTypedArrayType::JsArrayTypeUint8, nullptr, 0, &v);
    return v;
}

void *ChakraBackend::ArrayBuffer(uint32_t length) {
    JsValueRef v;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreateArrayBuffer), length, &v);
    return v;
}

void *ChakraBackend::GetTypedArrayData(void *value) {
    ChakraBytePtr chakraBytePtr = nullptr;
    unsigned int bufferLength = 0;
    JsTypedArrayType typedArrayType = JsTypedArrayType::JsArrayTypeFloat32;
    int elementSize = 0;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetTypedArrayStorage), value, &chakraBytePtr,
             &bufferLength, &typedArrayType, &elementSize);
    return chakraBytePtr;
}

uint32_t ChakraBackend::GetTypedArrayBufferLength(void *value) {
    ChakraBytePtr chakraBytePtr = nullptr;
    unsigned int bufferLength = 0;
    JsTypedArrayType typedArrayType = JsTypedArrayType::JsArrayTypeFloat32;
    int elementSize = 0;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetTypedArrayStorage), value, &chakraBytePtr,
             &bufferLength, &typedArrayType, &elementSize);
    return bufferLength;
}

void *ChakraBackend::GetArrayBufferData(void *value) {
    ChakraBytePtr chakraBytePtr = nullptr;
    unsigned int bufferLength = 0;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetArrayBufferStorage), value, &chakraBytePtr,
             &bufferLength);
    return chakraBytePtr;
}

uint32_t ChakraBackend::GetArrayBufferLength(void* value)
  {
    ChakraBytePtr chakraBytePtr = nullptr;
    unsigned int bufferLength = 0;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetArrayBufferStorage), value, &chakraBytePtr,
             &bufferLength);
    return bufferLength;
  }

void* ChakraBackend::ConvertValueToString(void *value) {
  JsValueRef res;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsConvertValueToString), value, &res);
    return res;
}

std::string ChakraBackend::GetString(void* value) {
  size_t outLength;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCopyString), value, nullptr, 0, &outLength);

    std::string res;
    res.resize(outLength);
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCopyString), value, res.data(), outLength,
             &outLength);
    return res;
}

bool ChakraBackend::GetBool(void *value) {
    bool res;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsBooleanToBool), value, &res);
    return res;
}

int ChakraBackend::GetInt(void *value) {
    int res;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsNumberToInt), value, &res);
    return res;
}

double ChakraBackend::GetDouble(void *value) {
    double res;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsNumberToDouble), value, &res);
    return res;
}

JsType ChakraBackend::GetType(void *value) {
    JsValueType type;
    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetValueType), value, &type);
    switch (type) {
        case JsUndefined:
            return JsType::Undefined;
        case JsNull:
            return JsType::Null;
        case JsNumber:
            return JsType::Number;
        case JsString:
            return JsType::String;
        case JsBoolean:
            return JsType::Boolean;
        case JsObject:
            return JsType::Object;
        case JsFunction:
            return JsType::Function;
        case JsError:
            return JsType::Error;
        case JsArray:
            return JsType::Array;
        case JsSymbol:
            return JsType::Symbol;
        case JsArrayBuffer:
            return JsType::ArrayBuffer;
        case JsTypedArray:
            return JsType::TypedArray;
        case JsDataView:
            return JsType::DataView;
        default:
            return JsType::Undefined;
    }
}

JsExternalObjectBase *ChakraBackend::GetExternalData(void *value) {
    void* externalData;
    bool hasExternslData;

    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsHasExternalData), value, &hasExternslData);
    if (!hasExternslData) {
      return nullptr;
    }

    ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsGetExternalData), value, &externalData);
    return reinterpret_cast<JsExternalObjectBase*>(externalData);
}

void ChakraBackend::SetProperty(void *value, void* key, void *newValue) {
  JsType type = GetType(key);
  switch (type) {
    case JsType::Number: {
      ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsSetIndexedProperty), value, key, newValue);
    } break;
    case JsType::String: {
      auto str = GetString(key);
      JsPropertyIdRef propId;
      ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsCreatePropertyId), str.data(), str.size(),
                  &propId);
      ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsSetProperty), value, propId, newValue,
                  true);
    } break;
    default:
      throw std::runtime_error("SetProperty: Bad key type (" +
                               std::to_string(int(type)) + ")");
  }
}

void ChakraBackend::DefineProperty(void *value, void* key, void* descriptor) {
  bool result;
  ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsObjectDefineProperty), value, key, descriptor, &result);
}

void ChakraBackend::AddRef(void *value) {
  ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsAddRef), value, nullptr);
}

void ChakraBackend::Release(void *value) {
  ChakraBackendUtils::SafeCall(JS_ENGINE_F(JsRelease), value, nullptr);
}
