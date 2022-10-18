#include "CardObj.h"

CardObj::CardObj(float length, int type) {
    this->setSize(Vector2f(length, length));
    side_length = length;
    card_type = type;
    this->setOutlineThickness(2);
    this->setOutlineColor(Color::Black);
}

void CardObj::set_card_id(int num) {
    id = num;
}

void CardObj::set_card_storey(int s) {
    storey = s;
}

void CardObj::set_card_position(int row, int column, int h_offset, int v_offset, int h_space, int v_space) {
    for (int i = 0; i != row; ++i) {
        for (int j = 0; j != column; ++j)
            setPosition((float)(h_offset * i + h_space * (i + 1)), v_space * (j + 1) + v_offset * j);
    }
}

int CardObj::get_card_storey() {
    return storey;
}


int CardObj::get_card_type() {
    return card_type;
}

int CardObj::get_card_id() {
    return id;
}

bool CardObj::set_card_availability(bool attr) {
    can_click = attr;
    return attr;
}

bool CardObj::get_card_availability() {
    return can_click;
}