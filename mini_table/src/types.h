struct vec {
    int x;
    int y;
};

struct color_rgb {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct pongBall {
    float speed = 1.0;
    float boost = 1.0;
    struct vec pos;
    struct vec dir;
    struct color_rgb color;
};

struct pongPlayer {
    int number;
    int size = 1;
    int score = 0;
    struct vec pos;
    struct color_rgb color;
    int playerType = 0;
    int wheelPort;
    int buttonPort;

};