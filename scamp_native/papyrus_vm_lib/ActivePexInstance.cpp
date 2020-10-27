#include "OpcodesImplementation.h"
#include "Utils.h"
#include "VirtualMachine.h"
#include <algorithm>
#include <cctype> // tolower
#include <functional>
#include <stdexcept>

ActivePexInstance::ActivePexInstance()
{
  this->parentVM = nullptr;
}

ActivePexInstance::ActivePexInstance(
  PexScript::Lazy sourcePex,
  const std::shared_ptr<IVariablesHolder>& mapForFillPropertys,
  VirtualMachine* parentVM, VarValue activeInstanceOwner,
  std::string childrenName)
{
  this->childrenName = childrenName;
  this->activeInstanceOwner = activeInstanceOwner;
  this->parentVM = parentVM;
  this->sourcePex = sourcePex;
  this->parentInstance =
    FillParentInstanse(sourcePex.fn()->objectTable.m_data[0].parentClassName,
                       activeInstanceOwner, mapForFillPropertys);

  this->variables = mapForFillPropertys;

  this->_IsValid = true;
}

ActivePexInstance::Ptr ActivePexInstance::FillParentInstanse(
  std::string nameNeedScript, VarValue activeInstanceOwner,
  const std::shared_ptr<IVariablesHolder>& mapForFillPropertys)
{
  return parentVM->CreateActivePexInstance(nameNeedScript, activeInstanceOwner,
                                           mapForFillPropertys,
                                           this->sourcePex.source);
}

FunctionInfo ActivePexInstance::GetFunctionByName(const char* name,
                                                  std::string stateName) const
{

  FunctionInfo function;
  for (auto& object : sourcePex.fn()->objectTable.m_data) {
    for (auto& state : object.states) {
      if (state.name == stateName) {
        for (auto& func : state.functions) {
          if (!Utils::stricmp(func.name.data(), name)) {
            function = func.function;
            function.valid = true;
            return function;
          }
        }
      }
    }
  }
  return function;
}

std::string ActivePexInstance::GetActiveStateName() const
{
  auto var = variables->GetVariableByName("::State", *sourcePex.fn());
  if (!var)
    throw std::runtime_error(
      "'::State' variable doesn't exist in ActivePexInstance");
  return static_cast<const char*>(*var);
}

ObjectTable::Object::PropInfo* ActivePexInstance::GetProperty(
  const ActivePexInstance& scriptInstance, std::string nameProperty,
  uint8_t flag)
{
  if (!scriptInstance.IsValid())
    return nullptr;

  if (flag == ObjectTable::Object::PropInfo::kFlags_Read) {

    for (auto& object : scriptInstance.sourcePex.fn()->objectTable.m_data) {
      for (auto& prop : object.properties) {
        if (prop.name == nameProperty &&
            (prop.flags & 5) == prop.kFlags_Read) {
          return &prop;
        }
      }
    }

    if (flag == ObjectTable::Object::PropInfo::kFlags_Write) {

      for (auto& object : scriptInstance.sourcePex.fn()->objectTable.m_data) {
        for (auto& prop : object.properties) {
          if (prop.name == nameProperty &&
              (prop.flags & 6) == prop.kFlags_Write) {
            return &prop;
          }
        }
      }
    }
  }

  return nullptr;
}

ActivePexInstance& ActivePexInstance::GetActivePexInObject(
  VarValue* object, std::string& scriptType)
{
  return parentVM->GetActivePexInObject(object, scriptType);
}

const std::string& ActivePexInstance::GetSourcePexName() const
{
  if (!sourcePex.fn()) {
    static const std::string empty = "";
    return empty;
  }

  return sourcePex.fn()->source;
}

VarValue ActivePexInstance::CastToString(const VarValue& var)
{
  std::string temp;
  size_t _size = this->sourcePex.fn()->stringTable.m_data.size();

  switch (var.GetType()) {

    case var.kType_Object: {
      IGameObject* ptr = ((IGameObject*)var);
      if (ptr)
        return VarValue(ptr->GetStringID());
      else {
        const static std::string noneString = "None";
        return VarValue(noneString.c_str());
      }
    }
    case var.kType_Identifier:
      assert(false);
      return VarValue();

    case var.kType_Integer:

      temp = std::to_string((int)var);
      for (auto& str : this->sourcePex.fn()->stringTable.m_data) {
        if (str.data() == temp) {
          return VarValue(str.data());
        }
      }

      this->sourcePex.fn()->stringTable.m_data.push_back(temp);
      return VarValue(this->sourcePex.fn()->stringTable.m_data[_size].data());

    case var.kType_Float:

      temp = std::to_string((float)var);
      for (auto& str : this->sourcePex.fn()->stringTable.m_data) {
        if (str.data() == temp) {
          return VarValue(str.data());
        }
      }

      this->sourcePex.fn()->stringTable.m_data.push_back(temp);
      return VarValue(this->sourcePex.fn()->stringTable.m_data[_size].data());

    case var.kType_Bool: {

      const static std::string boolTrue = "True";
      const static std::string boolFalse = "False";

      temp = std::to_string((bool)var);
      if (temp == "0")
        return VarValue(boolFalse.c_str());
      else
        return VarValue(boolTrue.c_str());
    }
    case var.kType_ObjectArray:
      return GetElementsArrayAtString(var, var.kType_ObjectArray);

    case var.kType_StringArray:
      return GetElementsArrayAtString(var, var.kType_StringArray);

    case var.kType_IntArray:
      return GetElementsArrayAtString(var, var.kType_IntArray);

    case var.kType_FloatArray:
      return GetElementsArrayAtString(var, var.kType_FloatArray);

    case var.kType_BoolArray:
      return GetElementsArrayAtString(var, var.kType_BoolArray);

    default:
      assert(false);
      return VarValue();
  }
}

VarValue ActivePexInstance::GetElementsArrayAtString(const VarValue& array,
                                                     uint8_t type)
{
  std::string returnValue = "[";

  for (size_t i = 0; i < array.pArray->size(); ++i) {
    // returnValue += " ";

    switch (type) {
      case array.kType_ObjectArray:
        returnValue += ((IGameObject*)((*array.pArray)[i]))->GetStringID();
        break;

      case array.kType_StringArray:
        returnValue += (const char*)((*array.pArray)[i]);
        break;

      case array.kType_IntArray:
        returnValue += std::to_string((int)((*array.pArray)[i]));
        break;

      case array.kType_FloatArray:
        returnValue += std::to_string((float)((*array.pArray)[i]));
        break;

      case array.kType_BoolArray: {
        VarValue& temp = ((*array.pArray)[i]);
        returnValue += (const char*)(CastToString(temp));
        break;
      }
      default:
        assert(false);
    }

    if (i < array.pArray->size() - 1)
      returnValue += " , ";
    else
      returnValue += "]";
  }

  instanceStringTable.push_back(std::make_shared<std::string>(returnValue));
  return VarValue(
    instanceStringTable[instanceStringTable.size() - 1]->c_str());
}

struct ActivePexInstance::ExecutionContext
{
  std::shared_ptr<Locals> locals;
  bool needReturn = false;
  bool needJump = false;
  int jumpStep = 0;
  VarValue returnValue = VarValue::None();
};

std::vector<VarValue> GetArgsForCall(uint8_t op,
                                     const std::vector<VarValue*>& opcodeArgs)
{
  std::vector<VarValue> argsForCall;
  if (opcodeArgs.size() > 4) {
    for (size_t i = 4; i < opcodeArgs.size(); ++i) {
      argsForCall.push_back(*opcodeArgs[i]);
    }
  }
  if (op == FunctionCode::kOp_CallParent && opcodeArgs.size() > 3) {
    for (size_t i = 3; i < opcodeArgs.size(); ++i) {
      argsForCall.push_back(*opcodeArgs[i]);
    }
  }
  return argsForCall;
}

void ActivePexInstance::ExecuteOpCode(ExecutionContext* ctx, uint8_t op,
                                      const std::vector<VarValue*>& args)
{
  auto argsForCall = GetArgsForCall(op, args);

  switch (op) {
    case OpcodesImplementation::Opcodes::op_Nop:
      break;
    case OpcodesImplementation::Opcodes::op_iAdd:
      *args[0] = *args[1] + (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_fAdd:
      *args[0] = *args[1] + (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_iSub:
      *args[0] = *args[1] - (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_fSub:
      *args[0] = *args[1] - (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_iMul:
      *args[0] = *args[1] * (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_fMul:
      *args[0] = *args[1] * (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_iDiv:
      *args[0] = *args[1] / (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_fDiv:
      *args[0] = *args[1] / (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_iMod:
      *args[0] = *args[1] % (*args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_Not:
      *args[0] = !(*args[1]);
      break;
    case OpcodesImplementation::Opcodes::op_iNeg:
      *args[0] = *args[1] * VarValue(-1);
      break;
    case OpcodesImplementation::Opcodes::op_fNeg:
      *args[0] = *args[1] * VarValue(-1.0f);
      break;
    case OpcodesImplementation::Opcodes::op_Assign:
      *args[0] = *args[1];
      break;
    case OpcodesImplementation::Opcodes::op_Cast:
      switch ((*args[0]).GetType()) {
        case VarValue::kType_Object:

          CastObjectToObject(args[0], args[1], *ctx->locals);
          break;
        case VarValue::kType_Integer:
          *args[0] = (*args[1]).CastToInt();
          break;
        case VarValue::kType_Float:
          *args[0] = (*args[1]).CastToFloat();
          break;
        case VarValue::kType_Bool:
          *args[0] = (*args[1]).CastToBool();
          break;
        case VarValue::kType_String:
          *args[0] = CastToString(*args[1]);
          break;
        default:
          assert(0);
      }
      break;
    case OpcodesImplementation::op_Cmp_eq:
      *args[0] = VarValue((*args[1]) == (*args[2]));
      break;
    case OpcodesImplementation::Opcodes::op_Cmp_lt:
      *args[0] = VarValue(*args[1] < *args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_Cmp_le:
      *args[0] = VarValue(*args[1] <= *args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_Cmp_gt:
      *args[0] = VarValue(*args[1] > *args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_Cmp_ge:
      *args[0] = VarValue(*args[1] >= *args[2]);
      break;
    case OpcodesImplementation::Opcodes::op_Jmp:
      ctx->jumpStep = (int)(*args[0]) - 1;
      ctx->needJump = true;
      break;
    case OpcodesImplementation::Opcodes::op_Jmpt:
      if ((bool)(*args[0])) {
        ctx->jumpStep = (int)(*args[1]) - 1;
        ctx->needJump = true;
      }
      break;
    case OpcodesImplementation::Opcodes::op_Jmpf:
      if ((bool)(!(*args[0]))) {
        ctx->jumpStep = (int)(*args[1]) - 1;
        ctx->needJump = true;
      }
      break;
    case OpcodesImplementation::Opcodes::op_CallMethod: {
      VarValue* object = args[1];
      std::string functionName = (const char*)(*args[0]);
      static const std::string nameOnBeginState = "onBeginState";
      static const std::string nameOnEndState = "onEndState";
      if (functionName == nameOnBeginState || functionName == nameOnEndState) {
        parentVM->SendEvent(this, functionName.c_str(), argsForCall);
        break;
      } else {
        auto res = parentVM->CallMethod(GetSourcePexName(), object,
                                        functionName.c_str(), argsForCall);
        if (res.promise) {
          res.promise->Then([](VarValue res) {
            // ...
          });
        }
        *args[2] = res;
      }
    } break;
    case OpcodesImplementation::Opcodes::op_CallParent: {
      const std::string& parentName =
        parentInstance ? parentInstance->GetSourcePexName() : "";
      *args[1] = parentVM->CallMethod(parentName, &activeInstanceOwner,
                                      (const char*)(*args[0]), argsForCall);
      break;
    }
    case OpcodesImplementation::Opcodes::op_CallStatic: {
      const char* className = (const char*)(*args[0]);
      const char* functionName = (const char*)(*args[1]);
      *args[2] = parentVM->CallStatic(className, functionName, argsForCall);
    } break;
    case OpcodesImplementation::Opcodes::op_Return:
      ctx->returnValue = *args[0];
      ctx->needReturn = true;
      break;
    case OpcodesImplementation::Opcodes::op_StrCat:
      OpcodesImplementation::strCat(*args[0], *args[1], *args[2],
                                    this->sourcePex.fn()->stringTable);
      break;
    case OpcodesImplementation::Opcodes::op_PropGet:
      if (args[1] != nullptr) {
        std::string nameProperty = (const char*)*args[0];
        auto& ptrPex = GetActivePexInObject(args[1], args[1]->objectType);
        ObjectTable::Object::PropInfo* runProperty = GetProperty(
          ptrPex, nameProperty, ObjectTable::Object::PropInfo::kFlags_Read);
        if (runProperty != nullptr) {
          *args[2] =
            ptrPex.StartFunction(runProperty->readHandler, argsForCall);
        }
      } else
        assert(false);
      break;
    case OpcodesImplementation::Opcodes::op_PropSet:
      if (args[1] != nullptr) {
        argsForCall.push_back(*args[2]);
        std::string nameProperty = (const char*)*args[0];
        auto& ptrPex = GetActivePexInObject(args[1], args[1]->objectType);
        ObjectTable::Object::PropInfo* runProperty = GetProperty(
          ptrPex, nameProperty, ObjectTable::Object::PropInfo::kFlags_Write);
        if (runProperty != nullptr) {
          ptrPex.StartFunction(runProperty->writeHandler, argsForCall);
        }
      } else
        assert(false);
      break;
    case OpcodesImplementation::Opcodes::op_Array_Create:
      (*args[0]).pArray =
        std::shared_ptr<std::vector<VarValue>>(new std::vector<VarValue>);
      if ((int32_t)(*args[1]) > 0) {
        (*args[0]).pArray->resize((int32_t)(*args[1]));
        uint8_t type = GetArrayElementType((*args[0]).GetType());
        for (auto& element : *(*args[0]).pArray) {
          element = VarValue(type);
        }
      } else
        assert(0);
      break;
    case OpcodesImplementation::Opcodes::op_Array_Length:
      if ((*args[1]).pArray != nullptr) {
        if ((*args[0]).GetType() == VarValue::kType_Integer)
          *args[0] = VarValue((int32_t)(*args[1]).pArray->size());
      } else
        *args[0] = VarValue((int32_t)0);
      break;
    case OpcodesImplementation::Opcodes::op_Array_GetElement:
      if ((*args[1]).pArray != nullptr) {
        *args[0] = (*args[1]).pArray->at((int32_t)(*args[2]));
      } else
        assert(0);
      break;
    case OpcodesImplementation::Opcodes::op_Array_SetElement:
      if ((*args[0]).pArray != nullptr) {
        (*args[0]).pArray->at((int32_t)(*args[1])) = *args[2];
      } else
        assert(0);
      break;
    case OpcodesImplementation::Opcodes::op_Array_FindElement:
      OpcodesImplementation::arrayFindElement(*args[0], *args[1], *args[2],
                                              *args[3]);
      break;
    case OpcodesImplementation::Opcodes::op_Array_RfindElement:
      OpcodesImplementation::arrayRFindElement(*args[0], *args[1], *args[2],
                                               *args[3]);
      break;
    default:
      assert(0);
  }
}

std::shared_ptr<ActivePexInstance::Locals> ActivePexInstance::MakeLocals(
  FunctionInfo& function, std::vector<VarValue>& arguments)
{
  auto locals = std::make_shared<Locals>();

  // Fill with function locals
  for (auto& var : function.locals) {
    VarValue temp = VarValue(GetTypeByName(var.type));
    temp.objectType = var.type;
    locals->push_back({ var.name, temp });
  }

  // Fill with function args
  for (size_t i = 0; i < arguments.size(); ++i) {
    VarValue temp = arguments[i];
    temp.objectType = function.params[i].type;

    locals->push_back({ function.params[i].name, temp });
    assert(locals->back().second.GetType() == arguments[i].GetType());
  }

  // ?
  for (size_t i = arguments.size(); i < function.params.size(); ++i) {
    const auto& var_ = function.params[i];

    VarValue temp = VarValue(GetTypeByName(var_.type));
    temp.objectType = var_.type;

    locals->push_back({ var_.name, temp });
  }

  // Dereference identifiers
  for (auto& var : *locals) {
    var.second = GetIndentifierValue(*locals, var.second);
  }

  return locals;
}

namespace {
// Basically, makes vector<VarValue *> from vector<VarValue>
std::vector<std::pair<uint8_t, std::vector<VarValue*>>> TransformInstructions(
  std::vector<FunctionCode::Instruction>& sourceOpCode)
{
  std::vector<std::pair<uint8_t, std::vector<VarValue*>>> opCode;
  for (size_t i = 0; i < sourceOpCode.size(); ++i) {

    std::pair<uint8_t, std::vector<VarValue*>> temp;
    temp.first = sourceOpCode[i].op;

    for (size_t j = 0; j < sourceOpCode[i].args.size(); ++j) {
      temp.second.push_back(&sourceOpCode[i].args[j]);
    }
    opCode.push_back(temp);
  }
  return opCode;
}
}

VarValue ActivePexInstance::StartFunction(FunctionInfo& function,
                                          std::vector<VarValue>& arguments)
{
  auto locals = MakeLocals(function, arguments);
  auto opCode = TransformInstructions(function.code.instructions);

  // Dereference identifiers
  for (auto& op : opCode)
    for (auto& arg : op.second)
      arg = &(GetIndentifierValue(*locals, *arg));

  ExecutionContext ctx{ locals };

  for (size_t line = 0; line < opCode.size(); ++line) {

    ExecuteOpCode(&ctx, opCode[line].first, opCode[line].second);

    if (ctx.needReturn) {
      ctx.needReturn = false;
      return ctx.returnValue;
    }

    if (ctx.needJump) {
      ctx.needJump = false;
      line += ctx.jumpStep;
    }
  }
  return ctx.returnValue;
}

VarValue& ActivePexInstance::GetIndentifierValue(Locals& locals,
                                                 VarValue& value)
{
  if (value.GetType() == VarValue::kType_Identifier &&
      (const char*)value != nullptr) {
    std::string temp;
    temp = temp + (const char*)value;
    return GetVariableValueByName(&locals, temp);
  } else
    return value;
}

uint8_t ActivePexInstance::GetTypeByName(std::string typeRef)
{

  std::transform(typeRef.begin(), typeRef.end(), typeRef.begin(), tolower);

  if (typeRef == "int") {
    return VarValue::kType_Integer;
  }
  if (typeRef == "float") {
    return VarValue::kType_Float;
  }
  if (typeRef == "string") {
    return VarValue::kType_String;
  }
  if (typeRef == "bool") {
    return VarValue::kType_Bool;
  }
  if (typeRef == "identifier") {
    assert(0);
  }
  if (typeRef == "string[]") {
    return VarValue::kType_StringArray;
  }
  if (typeRef == "int[]") {
    return VarValue::kType_IntArray;
  }
  if (typeRef == "float[]") {
    return VarValue::kType_FloatArray;
  }
  if (typeRef == "bool[]") {
    return VarValue::kType_BoolArray;
  }
  if (typeRef.find("[]") != std::string::npos) {
    return VarValue::kType_ObjectArray;
  }
  if (typeRef == "none") {
    // assert(false);
    return VarValue::kType_Object;
  }
  return VarValue::kType_Object;
}

uint8_t ActivePexInstance::GetArrayElementType(uint8_t type)
{
  uint8_t returnType;

  switch (type) {
    case VarValue::kType_ObjectArray:
      returnType = VarValue::kType_Object;

      break;
    case VarValue::kType_StringArray:
      returnType = VarValue::kType_String;

      break;
    case VarValue::kType_IntArray:
      returnType = VarValue::kType_Integer;

      break;
    case VarValue::kType_FloatArray:
      returnType = VarValue::kType_Float;

      break;
    case VarValue::kType_BoolArray:
      returnType = VarValue::kType_Bool;

      break;
    default:
      assert(false);
  }

  return returnType;
}

void ActivePexInstance::CastObjectToObject(VarValue* result,
                                           VarValue* scriptToCastOwner,
                                           Locals& locals)
{
  std::string objectToCastTypeName = scriptToCastOwner->objectType;
  std::string resultTypeName = result->objectType;

  auto object = static_cast<IGameObject*>(*scriptToCastOwner);
  if (object) {
    std::string scriptName = object->GetParentNativeScript();
    while (1) {
      if (scriptName.empty()) {
        break;
      }

      if (!Utils::stricmp(resultTypeName.data(), scriptName.data())) {
        *result = *scriptToCastOwner;
        return;
      }

      // TODO: Test this with attention
      // Here is the case when i.e. variable with type 'Form' casts to
      // 'ObjectReference' while it's actually an Actor

      auto myScriptPex = parentVM->GetPexByName(scriptName);

      if (!myScriptPex.fn) {
        break;
      }

      scriptName = myScriptPex.fn()->objectTable.m_data[0].parentClassName;
    }
  }

  *result = VarValue::None();
}

bool ActivePexInstance::HasParent(ActivePexInstance* script,
                                  std::string castToTypeName)
{

  if (script != nullptr) {

    if (script->sourcePex.fn()->source == castToTypeName)
      return true;

    if (script->parentInstance != nullptr &&
        script->parentInstance->sourcePex.fn()->source != "") {
      if (script->parentInstance->sourcePex.fn()->source == castToTypeName)
        return true;
      else
        return HasParent(script->parentInstance.get(), castToTypeName);
    }
  }

  return false;
}

bool ActivePexInstance::HasChild(ActivePexInstance* script,
                                 std::string castToTypeName)
{

  if (script != nullptr) {

    if (script->sourcePex.fn()->source == castToTypeName)
      return true;

    if (script->childrenName != "") {
      if (script->childrenName == castToTypeName)
        return true;
    }
  }
  return false;
}

VarValue& ActivePexInstance::GetVariableValueByName(Locals* locals,
                                                    std::string name)
{

  if (name == "self") {
    return activeInstanceOwner;
  }

  if (locals)
    for (auto& var : *locals) {
      if (var.first == name) {
        return var.second;
      }
    }

  auto var = this->variables->GetVariableByName(name.data(), *sourcePex.fn());
  if (var)
    return *var;

  for (auto& _name : identifiersValueNameCache) {
    if ((const char*)(*_name) == name) {
      return *_name;
    }
  }

  if (parentVM->IsNativeFunctionByNameExisted(GetSourcePexName())) {

    VarValue::Ptr functionName =
      std::make_shared<VarValue>((new std::string(name))->c_str());

    identifiersValueNameCache.push_back(functionName);
    return *functionName;
  }

  for (auto& _string : sourcePex.fn()->stringTable.m_data) {
    if (_string == name) {
      VarValue::Ptr stringTableValue =
        std::make_shared<VarValue>((new std::string(name))->c_str());

      identifiersValueNameCache.push_back(stringTableValue);
      return *stringTableValue;
    }
  }

  for (auto& _string : parentInstance->sourcePex.fn()->stringTable.m_data) {
    if (_string == name) {

      VarValue::Ptr stringTableParentValue =
        std::make_shared<VarValue>((new std::string(name))->c_str());

      identifiersValueNameCache.push_back(stringTableParentValue);
      return *stringTableParentValue;
    }
  }

  assert(false);
  static VarValue _;
  return _;
}