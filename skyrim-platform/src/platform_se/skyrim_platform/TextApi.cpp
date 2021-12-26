#include "TextApi.h"
#include <fstream>
#include <string>

namespace TextApi {
JsValue TextApi::CreateText(const JsFunctionArguments& args)
{
  std::array<double, 4> argColor;

  auto argPosX = static_cast<double>(args[1]);
  auto argPosY = static_cast<double>(args[2]);

  auto argString = static_cast<std::string>(args[3]);
  std::wstring argWstring(argString.begin(), argString.end());

  for (int i = 0; i < 4; i++) {
    argColor[i] = args[4].GetProperty(i);
  }

  return JsValue(TextsCollection::GetSinglton().CreateText(
    argPosX, argPosY, argWstring, argColor));
}

JsValue TextApi::DestroyText(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  TextsCollection::GetSinglton().DestroyText(argTextId);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextPos(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  auto argPosX = static_cast<double>(args[2]);
  auto argPosY = static_cast<double>(args[3]);

  TextsCollection::GetSinglton().SetTextPos(argTextId, argPosX, argPosY);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextString(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  auto argString = static_cast<std::string> (args[2]); 
  std::wstring argWstring(argString.begin(), argString.end());

  TextsCollection::GetSinglton().SetTextString(argTextId, argWstring);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextColor(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  std::array<double, 4> argColor;

  for (int i = 0; i < 4; i++) {
    argColor[i] = args[2].GetProperty(i);
  }

  TextsCollection::GetSinglton().SetTextColor(argTextId, argColor);
  return JsValue::Undefined();
}

JsValue TextApi::DestroyAllTexts()
{
  TextsCollection::GetSinglton().DestroyAllTexts();
  return JsValue::Undefined();
}

JsValue TextApi::GetTextPos(const JsFunctionArguments& args)
{
  //TextsCollection::GetSinglton().GetTextPos();
  return JsValue::Undefined();
}

JsValue TextApi::GetTextString(const JsFunctionArguments& args)
{
  std::wstring str =
    TextsCollection::GetSinglton().GetTextString(static_cast<int>(args[1]));

  return JsValue(std::string(str.begin(), str.end()));
}

JsValue TextApi::GetTextColor(const JsFunctionArguments& args)
{
  return JsValue::Undefined();
}

JsValue TextApi::GetCreatedTexts(const JsFunctionArguments& args)
{
  return JsValue::Undefined();
}

JsValue TextApi::GetTextCount(const JsFunctionArguments& args)
{
  return JsValue::Undefined();
}

void Register(JsValue& exports)
{
  exports.SetProperty(
    "createText",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return CreateText(args);
    }));

  exports.SetProperty(
    "destroyText",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return DestroyText(args);
    }));

  exports.SetProperty(
    "setTextPos",   
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return SetTextPos(args);
    }));

  exports.SetProperty(
    "setTextString",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return SetTextString(args);
    }));

  exports.SetProperty(
    "setTextColor",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return SetTextColor(args);
    }));

  exports.SetProperty(
    "destroyAllTexts",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return DestroyAllTexts();
    }));

  exports.SetProperty(
    "getTextString",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return GetTextString(args);
    }));
  
  exports.SetProperty(
    "getTextColor",
    JsValue::Function([=](const JsFunctionArguments& args) -> JsValue {
      return GetTextColor(args);
    }));
}

}