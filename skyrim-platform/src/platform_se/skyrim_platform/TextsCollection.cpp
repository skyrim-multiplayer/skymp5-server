#include "TextsCollection.h"
#include <DirectXTK/SpriteBatch.h>

TextsCollection::TextsCollection()
  : textCount(0)
{
}

TextsCollection::~TextsCollection()
{
}

int TextsCollection::CreateText(
  double xPos, double yPos, std::wstring string,
  std::array<double, 4> color = { 0.f, 0.f, 1.f, 1.f },
  std::string name = "Data/Platform/Fonts/Tavern.spritefont")
{
  TextToDraw text{ name, xPos, yPos, std::move(string), std::move(color) };

  textCount++;
  std::pair<int, TextToDraw> arg = { textCount, text };

  texts.insert(arg);

  return textCount;
}

void TextsCollection::DestroyText(int textId)
{
  texts.erase(textId);
}

void TextsCollection::SetTextPos(int textId, float xPos, float yPos)
{
  texts.at(textId).x = xPos;
  texts.at(textId).y = yPos;
}

void TextsCollection::SetTextString(int textId, std::wstring str)
{
  texts.at(textId).string = std::move(str);
}

void TextsCollection::SetTextColor(int textId, std::array<double, 4> color)
{
  texts.at(textId).color = color;
}

void TextsCollection::SetTextFont(int textId, std::string name)
{
  texts.at(textId).fontName = name;
}

void TextsCollection::SetTextRotation(int textId, float rotation)
{
  texts.at(textId).rotation = rotation;
}

void TextsCollection::SetTextSize(int textId, float size)
{
  texts.at(textId).size = size;
}

void TextsCollection::SetTextEffect(int textId, int effect)
{
  texts.at(textId).effects = static_cast<DirectX::SpriteEffects>(effect);
}

void TextsCollection::SetTextDepth(int textId, float Depth)
{
  texts.at(textId).layerDepth = Depth;
}

void TextsCollection::SetTextOrigin(int textId, std::array<double, 2> origin)
{
  texts.at(textId).origin = origin;
}

void TextsCollection::DestroyAllTexts()
{
  texts.clear();
  textCount = 0;
}

TextsCollection& TextsCollection::GetSingleton() noexcept
{
  static TextsCollection text;
  return text;
}

std::pair<double, double> TextsCollection::GetTextPos(int textId) const
{
  std::pair<double, double> positions = {
    texts.at(textId).x,
    texts.at(textId).y,
  };

  return positions;
}

const std::wstring& TextsCollection::GetTextString(int textId) const
{
  return texts.at(textId).string;
}

std::array<double, 4> TextsCollection::GetTextColor(int textId) const
{
  return texts.at(textId).color;
}

std::string TextsCollection::GetTextName(int textId) const
{
  std::string txt = texts.at(textId).fontName;

  return txt;
}

float TextsCollection::GetTextRotation(int textId) const
{
  return texts.at(textId).rotation;
}

float TextsCollection::GetTextSize(int textId) const
{
  return texts.at(textId).size;
}

int TextsCollection::GetTextEffect(int textId) const
{
  return texts.at(textId).effects;
}

int TextsCollection::GetTextDepth(int textId) const
{
  return texts.at(textId).layerDepth;
}

std::array<double, 2> TextsCollection::GetTextOrigin(int textId) const
{
  return std::array<double, 2>();
}

const std::unordered_map<int, TextToDraw>& TextsCollection::GetCreatedTexts()
  const
{
  return texts;
}
