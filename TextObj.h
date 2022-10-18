//
// Created by Tom Chu on 2022-10-15.
//

#include <SFML/Graphics.hpp>

using namespace sf;

#ifndef JLGJ_TEXTOBJ_H
#define JLGJ_TEXTOBJ_H


class TextObj : public Text {
public:
    TextObj(const Font& font, const wchar_t *str, const Color& color, const unsigned int size,
            const float pos_x, const float pos_y);

private:
};


#endif //JLGJ_TEXTOBJ_H
