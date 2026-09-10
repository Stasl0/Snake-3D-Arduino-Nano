#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// hardware definitions
#define LED_PIN 2
#define NUM_LEDS 125
#define UPDATE_INTERVAL 500 // milliseconds between snake movements

// button pins definitions
#define BTN_RESET 3
#define BTN_UP 4
#define BTN_DOWN 8
#define BTN_LEFT 7
#define BTN_RIGHT 6
#define BTN_FWD 5
#define BTN_BWD 9

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct {
    int x;
    int y;
    int z;
} pos;

// 3x5 font for numbers 0-9
const byte font_digits[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b010, 0b100, 0b100}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// global variables:
pos snake[125];
int snake_len;
int highscore = 0;
pos berry;
char direction;
char old_direction;
int score;
volatile bool reset_flag = false;
bool game_over = false;
unsigned long last_update_time = 0;

// colors definition
uint32_t color_white, color_black, color_snake, color_berry, color_head;
uint32_t snake_body[6];

// functions declarations:
void poll_buttons();
void snake_upd();
void spawn_berry();
void game_init();
void snake_init();
char snake_check();
int get_led_index(int x, int y, int z);
void draw_scene();
void draw_digit(int digit, int face, uint32_t color);
void display_score(int final_score);

void setup() {
    Serial.begin(9600);
    
    // init buttons
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_FWD, INPUT_PULLUP);
    pinMode(BTN_BWD, INPUT_PULLUP);
    pinMode(BTN_RESET, INPUT_PULLUP);
    
    // init LEDs
    pixels.begin();
    pixels.setBrightness(50);
    
    // define colors
    color_white = pixels.Color(255, 255, 255);
    color_black = pixels.Color(0, 0, 0);
    color_snake = pixels.Color(20, 20, 0);
    color_berry = pixels.Color(50, 0, 0);
    color_head = pixels.Color(0, 70, 0);
    
    snake_body[0] = pixels.Color(40, 40, 0);
    snake_body[1] = pixels.Color(25, 25, 0);
    snake_body[2] = pixels.Color(17, 17, 0);
    snake_body[3] = pixels.Color(8, 8, 0);
    snake_body[4] = pixels.Color(4, 4, 0);
    snake_body[5] = pixels.Color(1, 1, 0);
    
    // read highscore from EEPROM address 0
    EEPROM.get(0, highscore);
    
    // if EEPROM is empty (returns -1), set highscore to 0
    if (highscore < 0) {
        highscore = 0;
    }
    
    randomSeed(analogRead(0));
    game_init();
}

void loop() {
    poll_buttons();

    if (reset_flag) {
        reset_flag = false;
        game_init();
    }

    if (!game_over) {
        if (millis() - last_update_time >= UPDATE_INTERVAL) {
            last_update_time = millis();
            snake_upd();
            
            if (!snake_check()) {
                game_over = true;
                Serial.print("\nYou lost, score: ");
                Serial.println(score);
                Serial.print("Current highscore: ");
                Serial.println(highscore);
                
                if (score > highscore) {
                    highscore = score;
                    // save new highscore to EEPROM address 0
                    EEPROM.put(0, highscore);
                    
                    Serial.print("You got the new highscore! \nNew highscore: ");
                    Serial.println(highscore);
                }
                
                // display score on the LED cube faces
                display_score(score);
            } else {
                draw_scene();
            }
        }
    }
}

// maps 3D coordinates to 1D LED strip index based on column wiring
int get_led_index(int x, int y, int z) {
    int index = y * 25; // 25 LEDs per row (Y-axis)
    
    if (y % 2 == 0) {
        // even rows: left to right (X goes 0 to 4)
        index += x * 5;
    } else {
        // odd rows: right to left (X goes 4 to 0)
        index += (4 - x) * 5;
    }
    
    // every column starts at bottom (Z=0) and goes up
    index += z; 
    
    return index; 
}

void draw_scene() {
    pixels.clear(); // turn off all LEDs

    // draw berry
    pixels.setPixelColor(get_led_index(berry.x, berry.y, berry.z), color_berry);

    // draw body
    for (int i = 1; i < snake_len; i++) {
        uint32_t current_color = snake_body[i % 5];
        pixels.setPixelColor(get_led_index(snake[i].x, snake[i].y, snake[i].z), current_color);
    }
    
    // draw head
    pixels.setPixelColor(get_led_index(snake[0].x, snake[0].y, snake[0].z), color_head);
    
    pixels.show(); // update hardware
}

void poll_buttons() {
    // using INPUT_PULLUP, so LOW means pressed
    if (digitalRead(BTN_UP) == LOW && old_direction != 'd') direction = 'u';
    if (digitalRead(BTN_DOWN) == LOW && old_direction != 'u') direction = 'd';
    if (digitalRead(BTN_RIGHT) == LOW && old_direction != 'l') direction = 'r';
    if (digitalRead(BTN_LEFT) == LOW && old_direction != 'r') direction = 'l';
    if (digitalRead(BTN_FWD) == LOW && old_direction != 'b') direction = 'f';
    if (digitalRead(BTN_BWD) == LOW && old_direction != 'f') direction = 'b';
    
    if (digitalRead(BTN_RESET) == LOW) {
        reset_flag = true;
    }
}

void snake_upd() {
    pos head = snake[0];
    
    switch (direction) {
        case 'u': head.z++; if(head.z > 4) head.z = 0; break;
        case 'd': head.z--; if(head.z < 0) head.z = 4; break;
        case 'r': head.y++; if(head.y > 4) head.y = 0; break;
        case 'l': head.y--; if(head.y < 0) head.y = 4; break;
        case 'f': head.x++; if(head.x > 4) head.x = 0; break;
        case 'b': head.x--; if(head.x < 0) head.x = 4; break;
    }
    old_direction = direction;

    // shift body
    for (int i = snake_len; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = head;

    // check berry collision
    if (head.x == berry.x && head.y == berry.y && head.z == berry.z) {
        snake_len++; 
        spawn_berry(); 
        score++;
        Serial.print("Score: ");
        Serial.println(score);
    }
}

void spawn_berry() {
    int cross_out[125] = {0};

    // mark occupied LEDs
    for (int i = 0; i < snake_len; i++) {
        cross_out[get_led_index(snake[i].x, snake[i].y, snake[i].z)] = 1;
    }

    int random_number = random(1, (125 - snake_len) + 1);
    int p = 0; 
    
    // find free LED index
    while (random_number > 0) {
        random_number -= (1 - cross_out[p]);
        p++;
    }
    p--; 

    // extract 3D coordinates from 1D physical index based on column wiring
    berry.y = p / 25;
    int rem = p % 25;
    
    int col = rem / 5;
    berry.z = rem % 5;
    
    // apply serpentine correction for X coordinate
    if (berry.y % 2 == 0) {
        berry.x = col;
    } else {
        berry.x = 4 - col;
    }
}

char snake_check() {
    for (int i = 1; i < snake_len; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y && snake[0].z == snake[i].z) {
            return 0; // collision detected
        }
    }
    return 1; // clear
}

void game_init() {
    direction = 'f';
    old_direction = 'r';
    score = 0;
    reset_flag = false;
    game_over = false;
    
    snake_init();
    draw_scene();
    last_update_time = millis();
}

void snake_init() {
    snake_len = 5;
    snake[4] = (pos){0, 0, 0};
    snake[3] = (pos){0, 1, 0};
    snake[2] = (pos){0, 2, 0};
    snake[1] = (pos){0, 3, 0};
    snake[0] = (pos){0, 4, 0};
    
    berry = (pos){3, 4, 0};
}

// draw single digit on a specific wall
void draw_digit(int digit, int face, uint32_t color) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            // check if bit is set in the font array
            if (font_digits[digit][row] & (1 << (2 - col))) {
                int z = 4 - row; // invert row to match Z axis (0 is bottom)
                int offset = col + 1; // center digit (starts at index 1 instead of 0)
                
                if (face == 0) { 
                    // left face (X = 0)
                    // looking from outside, Y=4 is on the left and Y=0 is on the right
                    pixels.setPixelColor(get_led_index(0, 4 - offset, z), color);
                } else if (face == 1) { 
                    // front face (Y = 0)
                    // looking from outside, X=0 is on the left and X=4 is on the right
                    pixels.setPixelColor(get_led_index(offset, 0, z), color);
                } else if (face == 2) {
                    // right face (X = 4)
                    // looking from outside, Y=0 is on the left and Y=4 is on the right
                    pixels.setPixelColor(get_led_index(4, offset, z), color);
                }
            }
        }
    }
}

// clear screen and display final score on outer walls
void display_score(int final_score) {
    pixels.clear();
    
    if (final_score < 10) {
        // 1 digit
        draw_digit(final_score, 0, color_white);
    } else if (final_score < 100) {
        // 2 digits
        draw_digit(final_score / 10, 0, color_white); // left wall
        draw_digit(final_score % 10, 1, color_white); // front wall
    } else {
        // 3 digits (cap at 999)
        if (final_score > 999) final_score = 999;
        
        draw_digit((final_score / 100) % 10, 0, color_white); // left wall
        draw_digit((final_score / 10) % 10, 1, color_white);  // front wall
        draw_digit(final_score % 10, 2, color_white);         // right wall
    }
    
    pixels.show();
}