#include <SFML/Graphics.hpp>


using namespace sf;

class CardObj : public RectangleShape
{
public:
    CardObj();
    CardObj(float length, int type);


    friend bool operator <(const CardObj & obj1, const CardObj & obj2) {
        return obj1.card_type < obj2.card_type;
    }


    void set_card_id(int num);
    void set_card_storey(int s);
    void set_card_position(int row, int column, int h_os, int v_os, int h_sp, int v_space);

    int get_card_storey();
    int get_card_type();
    int get_card_id();

    bool set_card_availability(bool attr);
    bool get_card_availability();

private:
    float side_length;
    int card_type;
    int storey = 0;
    int id = -1;
    bool can_click = true;
};
