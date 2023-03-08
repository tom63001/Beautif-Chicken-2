#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CardObj.h"
#include "TextObj.h"
#include <random>
#include <cmath>
#include <algorithm>


using namespace std;
using namespace sf;

const int window_width = 1600;
const int window_height = 900;
const int brick_size = 40;
const int num_of_type = 7;
const float basketball_radius = 15.f;
const float bounding_box_radius = 4.f;

const float hp_proportion = 1000;

unsigned text_seed = std::chrono::system_clock::now().time_since_epoch().count();

//change according to window size, resolution adaption needs to be implemented.
const int starting_brick_pos_x = 80;
const int starting_brick_pos_y = 0;

vector<Texture> kunkun_frames;
vector<vector<Vector2f>> all_bb;
vector<Texture> brick_faces;
vector<Texture> basketball_frames;
Image bricks;

enum game_status {
    main_menu,
    in_game,
    win,
    fail
};

void add_texture(vector<CardObj>& brick_list) {
    for (auto& i : brick_list) {
        unsigned texture_seed = std::chrono::system_clock::now().time_since_epoch().count();
        i.setTexture(&brick_faces[texture_seed % num_of_type]);
    }
}

void add_color(vector<CardObj>& brick_list) {
    vector<CardObj*> temp;

    for (auto& i : brick_list) {
        temp.push_back(&i);
    }

    for (const auto &i : temp) {
        //unsigned texture_seed = std::chrono::system_clock::now().time_since_epoch().count();
        i->setFillColor(Color::Magenta);
    }
}


Texture pointer;

vector<CardObj> init_bricks() {
    Image brick_map;
    IntRect area;
    area.width = 40; area.height = 40;
    brick_map.loadFromFile("../Assets/bricks.bmp");
    vector<CardObj> rslt;

    pointer.create(40, 40);

    for (int j = 0; j < brick_map.getSize().y; j += 40) {
        for (int i = 0; i < brick_map.getSize().x; i += 40) {
            area.left = i; area.top = j;
            pointer.loadFromImage(brick_map, area);
            Image color_getter = pointer.copyToImage();
            auto color = color_getter.getPixel(10, 10);
            if (color.r == 0 || color.g == 0 || color.b == 0) {
                unsigned texture_seed = std::chrono::system_clock::now().time_since_epoch().count();
                CardObj temp_brick(brick_size, 1);

                temp_brick.setPosition((float)(i + starting_brick_pos_x), (float)(j + starting_brick_pos_y));
                temp_brick.setTexture(&brick_faces[texture_seed % num_of_type]);
                rslt.push_back(temp_brick);
            }
        }
    }

    return rslt;
}


Texture bb_pointer;
vector<Vector2f> init_bounding_box(string path) {
    Image obj;
    IntRect area;
    const float scan_size = 4;
    area.width = scan_size; area.height = scan_size;
    obj.loadFromFile(path);
    vector<Vector2f> rslt;
    bb_pointer.create(scan_size,scan_size);

    for (int j = 0; j < obj.getSize().y; j += scan_size) {
        for (int i = 0; i < obj.getSize().x; i += scan_size) {
            area.left = i; area.top = j;
            bb_pointer.loadFromImage(obj, area);
            Image color_getter = bb_pointer.copyToImage();
            auto color = color_getter.getPixel(0, 0);
            if (color.r < 255 || color.g < 255 || color.b < 255) {
                Vector2f temp(i + 800, j + 628);
                rslt.push_back(temp);
            }
        }
    }

    int first_row = rslt[0].y;
    int last_row = rslt[rslt.size() - 1].y;

    for (auto i = rslt.begin(); i != rslt.end(); ++i) {
        if(i->y > first_row && i->y < last_row) {
            while ((i - 1)->y == i->y && i->y == (i + 1)->y) {
                i = rslt.erase(i);
            }
        }

    }

    return rslt;
}


float distance_of_two_points(Vector2f A, Vector2f B) {
    return sqrt(((A.x - B.x) * (A.x - B.x)) + ((A.y - B.y) * (A.y - B.y)));
}

float dot_product(Vector2f A, Vector2f B) {
    return ((A.x * B.x) + (A.y * B.y));
}

float circle_line_collision2(Vector2f A, Vector2f B, Vector2f center) {
    const float l2 = brick_size * brick_size;
    //if (l2 == 0.0) return distance_of_two_points(A, center);
    const float t = max(0.f, min(1.f, dot_product(center - A, B - A) / l2));
    const Vector2f projection = A + t * (B - A);
    return distance_of_two_points(center, projection);
}

Vector2f kunkun_bouncing(CircleShape& current_ball, Vector2f basketball_pos, vector<Vector2f>& bounding_box, Vector2f velocity, int frame, RectangleShape& bar, Sound& body, Sound& head) {
    Vector2f rslt;
    bool collision = false;
    float center_x = basketball_pos.x + basketball_radius, center_y = basketball_pos.y + basketball_radius;
    float dist = basketball_radius + bounding_box_radius;
    for (auto i : bounding_box) {
        if (distance_of_two_points(basketball_pos, i) <= (dist + 0.f)) {
            //linear algebra content：
            float a1 = center_x, b1 = center_y;
            float a0 = i.x + (a1 - i.x) * (bounding_box_radius / (bounding_box_radius + basketball_radius)),
                    b0 = i.y + (b1 - i.y) * (bounding_box_radius / (bounding_box_radius + basketball_radius));
            float x0 = a0 + velocity.x, y0 = b0 + velocity.y;

            float x1 = (-2 / ((a1 - a0) * (a1 - a0) + (b1 - b0) * (b1 - b0))) *
                    ((a1 - a0) * (a1 - a0) * (a0 - x0) + (a1 - a0) * (b1 - b0) * (b0 - y0)) + (a0 - x0);

            float y1 = (-2 / ((a1 - a0) * (a1 - a0) + (b1 - b0) * (b1 - b0))) *
                    ((b1 - b0) * (a1 - a0) * (a0 - x0) + (b1 - b0) * (b1 - b0) * (b0 - y0)) + (b0 - y0);
            rslt.x = x1; rslt.y = y1;
            cout << rslt.x << " " << rslt.y << endl;
            collision = true;
            break;
        }
    }

    if (collision) {
        if (frame == 0 && center_y >= 673) {
            bar.setSize(Vector2f((bar.getSize().x - window_width / hp_proportion), bar.getSize().y));
            body.play();
        }
        if (frame == 0 && center_y >= 673 && bar.getSize().x <= window_width) {
            bar.setSize(Vector2f((bar.getSize().x + 5 * (window_width / hp_proportion)), bar.getSize().y));
            head.play();
        }


        if (frame == 1 && center_y >= 691) {
            bar.setSize(Vector2f((bar.getSize().x - window_width / hp_proportion), bar.getSize().y));
            body.play();
        }
        if (frame == 1 && center_y <= 691 && bar.getSize().x <= window_width) {
            bar.setSize(Vector2f((bar.getSize().x + 5 * (window_width / hp_proportion)), bar.getSize().y));
            head.play();
        }


        if (frame == 2 && center_y >= 658) {
            bar.setSize(Vector2f((bar.getSize().x - window_width / hp_proportion), bar.getSize().y));
            body.play();
        }
        if (frame == 2 && center_y <= 658 && bar.getSize().x <= window_width) {
            bar.setSize(Vector2f((bar.getSize().x + 5 * (window_width / hp_proportion)), bar.getSize().y));
            head.play();
        }


    }

    return rslt;
}



int main() {
    RenderWindow window(VideoMode(window_width, window_height), L"鸡了个鸡2 - 坤坤的反击");
    window.setFramerateLimit(200);

    game_status current_status = main_menu;

    bool is_ball_released = false;

    Texture bkg_image;
    bkg_image.loadFromFile("../Assets/bkg.png");
    RectangleShape background;
    background.setSize(Vector2f(window_width, window_height));
    background.setTexture(&bkg_image);


    for (int i = 0; i != num_of_type; ++i) {
        string file_name = "../Assets/card";
        Texture temp_tex;
        temp_tex.loadFromFile(file_name + to_string(i) + ".jpg");
        brick_faces.push_back(temp_tex);
    }

    for (int i = 0; i != 4; ++i) {
        string file_name = "../Assets/basketball";
        Texture temp_tex;
        temp_tex.loadFromFile(file_name + to_string(i) + ".png");
        basketball_frames.push_back(temp_tex);
    }

    vector<CardObj> all_bricks = init_bricks();


    RectangleShape hp_bkg, hp_bar;
    float bar_height = 100;
    hp_bkg.setSize(Vector2f(window_width, bar_height));
    hp_bar.setSize(Vector2f(window_width, bar_height));

    hp_bkg.setPosition(0, window_height - bar_height);
    hp_bar.setPosition(0, window_height - bar_height);

    hp_bkg.setOutlineColor(Color::Black);
    hp_bkg.setOutlineThickness(2);

    hp_bkg.setFillColor(Color(171,19,19));
    hp_bar.setFillColor(Color(28, 255, 49));


    RectangleShape kunkun;
    Texture kunkun_texture;
    Texture kunkun_texture2;
    Texture kunkun_texture3;


    kunkun_texture.loadFromFile("../Assets/kunkun.png");
    kunkun_texture2.loadFromFile("../Assets/kunkun2.png");
    kunkun_texture3.loadFromFile("../Assets/kunkun3.png");

    kunkun_frames.push_back(kunkun_texture);
    kunkun_frames.push_back(kunkun_texture2);
    kunkun_frames.push_back(kunkun_texture3);


    Vector2f kunkun_size(((float)window_width / 13.33f), ((float)window_height / 4.5f));
    kunkun.setSize(kunkun_size);
    kunkun.setPosition(800, 628);
    //kunkun.setFillColor(Color::Red);
    kunkun.setTexture(&kunkun_texture);


    vector<Vector2f> kunkun_bounding_box = init_bounding_box("../Assets/kunkun.bmp");
    vector<Vector2f> kunkun_bounding_box2 = init_bounding_box("../Assets/kunkun2.bmp");
    vector<Vector2f> kunkun_bounding_box3 = init_bounding_box("../Assets/kunkun3.bmp");

    all_bb.push_back(kunkun_bounding_box);
    all_bb.push_back(kunkun_bounding_box2);
    all_bb.push_back(kunkun_bounding_box3);

    vector<Vector2f> current_bounding_box = all_bb[0];

    vector<CircleShape> bounding_box_drawable;
    for (auto i : kunkun_bounding_box) {
        CircleShape temp;
        temp.setRadius(bounding_box_radius);
        temp.setPosition(i);
        temp.setFillColor(Color::Red);
        bounding_box_drawable.push_back(temp);
    }

    vector<CircleShape> bounding_box_drawable2;
    for (auto i : kunkun_bounding_box2) {
        CircleShape temp;
        temp.setRadius(bounding_box_radius);
        temp.setPosition(i);
        temp.setFillColor(Color::Red);
        bounding_box_drawable2.push_back(temp);
    }

    vector<CircleShape> bounding_box_drawable3;
    for (auto i : kunkun_bounding_box2) {
        CircleShape temp;
        temp.setRadius(bounding_box_radius);
        temp.setPosition(i);
        temp.setFillColor(Color::Red);
        bounding_box_drawable3.push_back(temp);
    }

    vector<vector<CircleShape> > all_bb_drawable;
    all_bb_drawable.push_back(bounding_box_drawable);
    all_bb_drawable.push_back(bounding_box_drawable2);
    all_bb_drawable.push_back(bounding_box_drawable3);

    vector<CircleShape> current_bb_drawable = all_bb_drawable[0];

    CircleShape basketball;
    Texture basketball_texture;
    basketball_texture.loadFromFile("../Assets/basketball0.png");
    basketball.setRadius(basketball_radius);
    basketball.setPosition(875,630);
    //basketball.setPosition(kunkun.getPosition().x, kunkun.getPosition().y - 2 * basketball_radius);
    //basketball.setFillColor(Color(231,120,0));
    basketball.setTexture(&basketball_texture);

    Font start_game_font;
    start_game_font.loadFromFile("../Assets/Hiragino Sans GB.ttc");

    vector<TextObj> win_text_arr;

    TextObj win_game_text0(start_game_font, L"练习时长两年半", Color::Black, 80, 200.f, 400.f);
    TextObj win_game_text1(start_game_font, L"唱，跳，rap，篮球", Color::Black, 80, 200.f, 400.f);
    TextObj win_game_text2(start_game_font, L"鸡你实在是太美", Color::Black, 80, 200.f, 400.f);
    win_text_arr.push_back(win_game_text0);
    win_text_arr.push_back(win_game_text1);
    win_text_arr.push_back(win_game_text2);
    shuffle(win_text_arr.begin(), win_text_arr.end(), std::default_random_engine(text_seed));

    TextObj fail_game_text0(start_game_font, L"你是真的蔡", Color::Black, 80, 200.f, 400.f);



    TextObj start_game_text(start_game_font, L"鸡了个鸡2： 坤坤的反击 \n按enter开始游戏", Color::Black, 70, 120.f, 300.f);
    TextObj copyright(start_game_font, L"BGM: Wang Rong Rolling - Chick Chick", Color::Black, 35, 120.f, 650.f);
    TextObj location_text(start_game_font, L"na, na", Color::Magenta, 15, 100.f, 500.f);
    TextObj mouse_text(start_game_font, L"na, na", Color::Black, 40, -100.f, -500.f);

    Music main_theme;
    main_theme.openFromFile("../Assets/jlgj2_main_theme.wav");
    main_theme.setVolume(15);
    main_theme.setLoop(true);
    main_theme.play();

    SoundBuffer floor_sound_buffer, bounce_effect_buffer, body_sound_buffer;
    floor_sound_buffer.loadFromFile("../Assets/erase0.wav");
    bounce_effect_buffer.loadFromFile("../Assets/bounce1.wav");
    body_sound_buffer.loadFromFile("../Assets//click0.wav");


    Sound floor_sound, bounce_sound, body_sound;
    floor_sound.setBuffer(floor_sound_buffer);
    floor_sound.setVolume(20);
    bounce_sound.setBuffer(bounce_effect_buffer);
    bounce_sound.setVolume(60);
    body_sound.setBuffer(body_sound_buffer);
    body_sound.setVolume(20);


    float basketball_deltax = 0.f;
    float basketball_deltay = 0.f;
    float kunkun_deltax = 0.f;
    bool pause = false;
    long double total_move = 0.f;

    int current_frame = 0;
    int current_frame2 = 0;

    int change_frame = 0;
    int change_frame2 = 0;

    Clock clock;

    //main game loop:
    while (window.isOpen()) {
        float elapsed = clock.restart().asMilliseconds();
        bool check = false;

        if (current_status == main_menu) {
            Event main_menu_event;
            while (window.pollEvent(main_menu_event)) {
                if (main_menu_event.type == Event::Closed)
                    window.close();
                if (main_menu_event.type == Event::KeyPressed) {
                    if (main_menu_event.key.code == Keyboard::Enter)
                        current_status = in_game;
                    if (main_menu_event.key.code == Keyboard::Escape)
                        window.close();
                }
            }
            window.clear(Color::Red);
            window.draw(background);
            window.draw(start_game_text);
            window.draw(copyright);
            window.display();
        }



        if (current_status == in_game) {
            if (!is_ball_released) {
                basketball_deltax = 0;
            }
            kunkun_deltax = 0;

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                    window.close();
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space && !is_ball_released) {
                    is_ball_released = true;
                    check = true;
                }
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::M)
                    pause = !pause;

                if (event.type == Event::MouseButtonPressed) {
                    if (event.key.code == Mouse::Left) {
                        mouse_text.setPosition(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
                        cout << "Mouse pressed at: " << Mouse::getPosition(window).x << " " << Mouse::getPosition(window).y << endl;
                        mouse_text.setString(to_string(Mouse::getPosition(window).x) + " " + to_string(Mouse::getPosition(window).y));
                    }
                }
            }

            if (!is_ball_released) {
                if (Keyboard::isKeyPressed(Keyboard::A) && kunkun.getPosition().x > 0) {
                    kunkun_deltax = -5.f;
                    basketball_deltax = -5.f;
                }
                if (Keyboard::isKeyPressed(Keyboard::D) && kunkun.getPosition().x < window_width - 120) {
                    kunkun_deltax = 5.f;
                    basketball_deltax = 5.f;
                }
            }
            else {
                if (Keyboard::isKeyPressed(Keyboard::A) && kunkun.getPosition().x > 0) {
                    kunkun_deltax = -5.f;

                }
                if (Keyboard::isKeyPressed(Keyboard::D) && kunkun.getPosition().x < window_width - 120) {
                    kunkun_deltax = 5.f;

                }
            }

            change_frame += elapsed;
            if (change_frame >= 120 && is_ball_released && !pause) {
                kunkun.setTexture(&kunkun_frames[current_frame]);
                current_bounding_box = all_bb[current_frame];
                ++current_frame;
                current_frame %= 3;
                change_frame = 0;
            }

            change_frame2 += elapsed;
            if (change_frame2 >= 60 && is_ball_released && !pause) {
                basketball.setTexture(&basketball_frames[current_frame2]);
                ++current_frame2;
                current_frame2 %= 4;
                change_frame2 = 0;
            }

            if (is_ball_released && check) {
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<float> dist(-2, 2);
                basketball_deltay = -3.5f;
                basketball_deltax = dist(gen);
            }


            //collision detection with walls & ceiling;
            if ((basketball.getPosition().x < 0 || basketball.getPosition().x + 2 * basketball_radius > window_width + 1) && is_ball_released) {
                basketball_deltax *= -1;
                bounce_sound.play();
            }


            if ((basketball.getPosition().y < 0 || basketball.getPosition().y + 2 * basketball_radius > window_height + 1) && is_ball_released) {
                basketball_deltay *= -1;
                bounce_sound.play();
            }

            if (basketball.getPosition().y + 2 * basketball_radius >= window_height) {
                floor_sound.play();
                bounce_sound.play();
                hp_bar.setSize(Vector2f((hp_bar.getSize().x - 40 * (window_width / hp_proportion)), hp_bar.getSize().y));
            }


            //collision detection with bricks;
            if (basketball.getPosition().y < 400.f && basketball.getPosition().x >= 0 &&
                basketball.getPosition().y >=0 && basketball.getPosition().x <= window_width && is_ball_released) {
                for (auto i = all_bricks.begin(); i != all_bricks.end(); ++i) {
                    Vector2f center(basketball.getPosition().x + basketball_radius, basketball.getPosition().y + basketball_radius);
                    Vector2f top_left = i->getPosition();
                    Vector2f top_right(top_left.x + brick_size, top_left.y);
                    Vector2f bottom_left(top_left.x, top_left.y + brick_size);
                    Vector2f bottom_right(top_left.x + brick_size, top_left.y + brick_size);

                    //the last int param indicates if there's a non-tangent collision, which intersecting point to be returned(1 or 2?).
                    //collision with left side:
                    if (circle_line_collision2(top_left, bottom_left, center) <= basketball_radius) {
                        i = all_bricks.erase(i);
                        basketball_deltax *= -1;
                        bounce_sound.play();
                        break;
                    }

                    //collision with top side:
                    if (circle_line_collision2(top_left, top_right, center) <= basketball_radius) {
                        i = all_bricks.erase(i);
                        basketball_deltay *= -1;
                        bounce_sound.play();
                        break;
                    }

                    //collision with right side:
                    if (circle_line_collision2(bottom_right, top_right, center) <= basketball_radius) {
                        i = all_bricks.erase(i);
                        basketball_deltax *= -1;
                        bounce_sound.play();
                        break;
                    }

                    //collision with bottom side:
                    if (circle_line_collision2(bottom_left, bottom_right, center) <= basketball_radius) {
                        i = all_bricks.erase(i);
                        basketball_deltay *= -1;
                        bounce_sound.play();
                        break;
                    }

                }
            }
            //collision detection with kunkun;
            Vector2f current_ball_pos = basketball.getPosition();

            if (current_ball_pos.y >= 400.f && current_ball_pos.x >= 0 &&
                    current_ball_pos.y <= window_height && current_ball_pos.x <= window_width && abs(total_move) >= 50) {
                Vector2f current_ball_velocity(basketball_deltax, basketball_deltay);
                Vector2f new_velocity = kunkun_bouncing(basketball, current_ball_pos, current_bounding_box, current_ball_velocity, current_frame, hp_bar, body_sound, bounce_sound);
                if ((new_velocity.x * new_velocity.x + new_velocity.y * new_velocity.y) > 0) {
                    basketball_deltax = new_velocity.x;
                    basketball_deltay = new_velocity.y;
                }
            }

            if (all_bricks.empty())
                current_status = win;
            if (hp_bar.getSize().x < 0)
                current_status = fail;

            if (!pause) {
                window.clear(Color::White);
                window.draw(background);
                location_text.setPosition(current_ball_pos.x, current_ball_pos.y);
                location_text.setString(to_string(current_ball_pos.x) + " " + to_string(current_ball_pos.y));
                basketball.setPosition(current_ball_pos.x + basketball_deltax, current_ball_pos.y + basketball_deltay);

                if (is_ball_released) {
                    total_move += basketball_deltax;
                    total_move += basketball_deltay;

                }

                for (auto i : all_bricks) {
                    window.draw(i);
                }


                kunkun.move(kunkun_deltax, 0);
                for (auto &i : all_bb) {
                    for (auto &j : i) {
                        j.x += kunkun_deltax;
                    }
                }

                for (int i = 0; i != current_bb_drawable.size(); ++i) {
                    current_bb_drawable[i].setPosition(current_bounding_box[i]);
                }

                window.draw(hp_bkg);
                window.draw(hp_bar);
                window.draw(kunkun);

                window.draw(basketball);

                window.display();
            }
            else {
                window.clear(Color::White);
                for (auto i : all_bricks) {
                    window.draw(i);
                }
                window.draw(hp_bkg);
                window.draw(hp_bar);
                window.draw(kunkun);
                window.draw(basketball);
                window.display();

            }
        }

        if (current_status == win) {
            Event win_event;
            while(window.pollEvent(win_event)) {
                if (win_event.type == Event::Closed)
                    window.close();
                if (win_event.type == Event::KeyPressed && win_event.key.code == Keyboard::Escape)
                    window.close();
            }
            window.clear(Color::White);
            window.draw(background);
            window.draw(win_text_arr[0]);
            window.display();
        }

        if (current_status == fail) {
            Event fail_event;
            while(window.pollEvent(fail_event)) {
                if (fail_event.type == Event::Closed)
                    window.close();
                if (fail_event.type == Event::KeyPressed && fail_event.key.code == Keyboard::Escape)
                    window.close();
            }
            window.clear(Color::White);
            window.draw(background);
            window.draw(fail_game_text0);
            window.display();
        }
    }

    return 0;
}
