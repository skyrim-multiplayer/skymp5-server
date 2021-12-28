#include "TextApi.h"
#include <fstream>
#include <string>

#include <codecvt>
#include <locale>

namespace TextApi {
JsValue TextApi::CreateText(const JsFunctionArguments& args)
{
  std::array<double, 4> argColor;

  auto argPosX = static_cast<double>(args[1]);
  auto argPosY = static_cast<double>(args[2]);
  auto argString = static_cast<std::string>(args[3]);

  for (int i = 0; i < 4; i++) {
    argColor[i] = args[4].GetProperty(i);
  }

  return JsValue(TextsCollection::GetSingleton().CreateText(
    argPosX, argPosY, argString, argColor));
}

JsValue TextApi::DestroyText(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  TextsCollection::GetSingleton().DestroyText(argTextId);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextPos(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  auto argPosX = static_cast<double>(args[2]);
  auto argPosY = static_cast<double>(args[3]);

  TextsCollection::GetSingleton().SetTextPos(argTextId, argPosX, argPosY);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextString(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  auto argString = static_cast<std::string> (args[2]); 

  TextsCollection::GetSingleton().SetTextString(argTextId, argString);
  return JsValue::Undefined();
}

JsValue TextApi::SetTextColor_(const JsFunctionArguments& args)
{
  auto argTextId = static_cast<int>(args[1]);
  std::array<double, 4> argColor;

  for (int i = 0; i < 4; i++) {
    argColor[i] = args[2].GetProperty(i);
  }

  TextsCollection::GetSingleton().SetTextColor(argTextId, argColor);
  return JsValue::Undefined();
}

JsValue TextApi::DestroyAllTexts(const JsFunctionArguments&)
{
  TextsCollection::GetSingleton().DestroyAllTexts();
  return JsValue::Undefined();
}

JsValue TextApi::GetTextPos(const JsFunctionArguments& args)
{
  auto argId = static_cast<int> (args[1]);
  auto postions = TextsCollection::GetSingleton().GetTextPos(argId);
  auto jsArray = JsValue::Array(2);

  jsArray.SetProperty(0, postions.first);
  jsArray.SetProperty(1, postions.second);

  return jsArray;
}

JsValue TextApi::GetTextString(const JsFunctionArguments& args)
{
  std::string str =
    TextsCollection::GetSingleton().GetTextString(static_cast<int>(args[1]));

  return JsValue(str);
}

JsValue TextApi::GetTextColor(const JsFunctionArguments& args)
{
  auto argId = static_cast<int>(args[1]);
  auto argArray = TextsCollection::GetSingleton().GetTextColor(argId);
  auto jsArray = JsValue::Array(4);

  for (int i = 0; i < 4; i++) {
    jsArray.SetProperty(i, argArray.at(i));
  }

  return jsArray;
}

JsValue TextApi::GetTextCount(const JsFunctionArguments& args)
{
  return JsValue(TextsCollection::GetSingleton().GetTextCount());
}

void Register(JsValue& exports)
{
  exports.SetProperty("createText", JsValue::Function(CreateText));

  exports.SetProperty("destroyText", JsValue::Function(DestroyText));

  exports.SetProperty("setTextPos", JsValue::Function(SetTextPos));

  exports.SetProperty("setTextString", JsValue::Function(SetTextString));

  exports.SetProperty("setTextColor", JsValue::Function(SetTextColor_));

  exports.SetProperty("destroyAllTexts", JsValue::Function(DestroyAllTexts));

  exports.SetProperty("getTextString", JsValue::Function(GetTextString));

  exports.SetProperty("getTextColor", JsValue::Function(GetTextColor));
}

}
