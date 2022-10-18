//
// Created by Tom Chu on 2022-10-15.
//

#include "TextObj.h"

TextObj::TextObj(const Font& font, const wchar_t *str, const Color& color, const unsigned int size,
                 const float pos_x, const float pos_y) {
    this->setFont(font);
    this->setString(str);
    this->setFillColor(color);
    this->setCharacterSize(size);
    this->setPosition(pos_x, pos_y);
}
