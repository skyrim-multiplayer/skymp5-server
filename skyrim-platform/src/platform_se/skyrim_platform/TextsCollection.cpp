#include "TextsCollection.h"

TextsCollection::TextsCollection()
{
  makeId.reset(new MakeID(std::numeric_limits<uint32_t>::max()));
}

TextsCollection::~TextsCollection()
{
}

int TextsCollection::CreateText(double xPos, double yPos, std::wstring str,
                                std::array<double, 4> color = { 0.f, 0.f, 1.f,
                                                                1.f },
                                std::wstring name = L"Tavern")
{
  TextToDraw text{ name, xPos, yPos, str, color };

  uint32_t id;
  makeId->CreateID(id);
  if (texts.size() <= id) {
    texts.resize(id);
  }

  texts[id] = text;
  return id;
}

void TextsCollection::DestroyText(int textId)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts.erase(texts.begin() + textId);
  makeId->DestroyID(textId);
}
void TextsCollection::DestroyAllTexts()
{
  texts.clear();
  makeId.reset(new MakeID(std::numeric_limits<uint32_t>::max()));
}

void TextsCollection::SetTextPos(int& textId, double& xPos, double& yPos)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].x = xPos;
  texts[textId].y = yPos;
}
void TextsCollection::SetTextString(int& textId, std::wstring& str)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].string = std::move(str);
}
void TextsCollection::SetTextColor(int& textId, std::array<double, 4>& color)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].color = color;
}
void TextsCollection::SetTextFont(int& textId, std::wstring& name)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].fontName = name;
}
void TextsCollection::SetTextRotation(int& textId, float& rotation)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].rotation = rotation;
}
void TextsCollection::SetTextSize(int& textId, float& size)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].size = size;
}
void TextsCollection::SetTextEffect(int& textId, int& effect)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].effects = static_cast<DirectX::SpriteEffects>(effect);
}
void TextsCollection::SetTextDepth(int& textId, int& depth)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].layerDepth = depth;
}
void TextsCollection::SetTextOrigin(int& textId, std::array<double, 2>& origin)
{
  if (makeId->IsID(textId) == false) {
    return;
  }
  texts[textId].origin = origin;
}

const std::pair<double, double>& TextsCollection::GetTextPos(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextPos - textId doesn't exist");
  }

  std::pair<double, double> positions = {
    texts[textId].x,
    texts[textId].y,
  };

  return positions;
}
const std::wstring& TextsCollection::GetTextString(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextString - textId doesn't exist");
  }
  return texts[textId].string;
}
const std::array<double, 4>& TextsCollection::GetTextColor(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextColor - textId doesn't exist");
  }
  return texts[textId].color;
}
const std::wstring& TextsCollection::GetTextFont(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextFont - textId doesn't exist");
  }
  return texts[textId].fontName;
}
const float& TextsCollection::GetTextRotation(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextRotation - textId doesn't exist");
  }
  return texts[textId].rotation;
}
const float& TextsCollection::GetTextSize(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextSize - textId doesn't exist");
  }
  return texts[textId].size;
}
const int& TextsCollection::GetTextEffect(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextEffect - textId doesn't exist");
  }
  return texts[textId].effects;
}
const int& TextsCollection::GetTextDepth(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextDepth - textId doesn't exist");
  }
  return texts[textId].layerDepth;
}
const std::array<double, 2>& TextsCollection::GetTextOrigin(int textId) const
{
  if (makeId->IsID(textId) == false) {
    throw std::runtime_error("GetTextOrigin - textId doesn't exist");
  }
  return texts[textId].origin;
}

const std::vector<TextToDraw>& TextsCollection::GetCreatedTexts() const
{
  return texts;
}

TextsCollection& TextsCollection::GetSingleton() noexcept
{
  static TextsCollection text;
  return text;
}
