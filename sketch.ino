#include <FastLED.h>
#include <LinkedList.h>

const int PIN_LED = 4;
const int PIN_BUTTON_P1 = 2;
const int PIN_BUTTON_P2 = 3;
const int LED_COUNT = 30;

const int LIFES = 5;
const int START_SPEED = 50;

CRGB leds[LED_COUNT] = {};

int pos = 15;
int direction = 1;
int speed = START_SPEED;
int speeds[10] = { 10, 30, 50, 100, 200, 200, 200, 200, 200, 200 };

int lastPressed = -1;

bool game_started = false;

LinkedList<int> barriers_p1 = LinkedList<int>();
LinkedList<int> barriers_p2 = LinkedList<int>();

void setup() {
  Serial.begin(9600);
  
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(150);

  for(int i = LED_COUNT - LIFES; i < LED_COUNT; i++) {
    barriers_p2.add(i);
  }

  for(int i = 0; i < LIFES; i++) {
    barriers_p1.add(i);
  }
  
  pinMode(PIN_BUTTON_P1, INPUT);
  pinMode(PIN_BUTTON_P2, INPUT);

  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, LED_COUNT);
}

void clear_strip() {
  FastLED.clear();
  FastLED.show();
}

void fill_strip(CRGB color) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void light_up(CRGB color) {
  clear_strip();
  fill_strip(color);
  delay(300);
}

void reset_game() {
  pos = 15;
  game_started = false;
  lastPressed = -1;
  speed = START_SPEED;
}

void p1_win_stip() {
  for(int i = 0; i <= 10; i++) {
    clear_strip();
    delay(500);
    for (int i = 0; i < LIFES; i++) {
      leds[i] = CRGB::Blue;
    }
    for (int i = LED_COUNT - LIFES; i < LED_COUNT; i++) {
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
  }
}

void p2_win_stip() {
  for(int i = 0; i <= 10; i++) {
    clear_strip();
    delay(500);
    for (int i = 0; i < LIFES; i++) {
      leds[i] = CRGB::Red;
    }
    for (int i = LED_COUNT - LIFES; i < LED_COUNT; i++) {
      leds[i] = CRGB::Blue;
    }
    FastLED.show();
    delay(500);
  }
}

void move() {
  if (direction == 2) {
    pos++;
  } else {
    pos--;
  }
  
  if (pos > 29) {
    direction = 1;
  }
  if (pos < 0) {
    direction = 2;
  }

  bool isPlaygroundExceededAtP1 = pos < 0;
  bool isPlaygroundExceededAtP2 = pos > LED_COUNT - 1;

  if(isPlaygroundExceededAtP1) {
    barriers_p1.add(barriers_p1.size());
    barriers_p2.remove(0);
    lastPressed = -1;
    speed = START_SPEED;
    light_up(CRGB::Red);
  }

  if(isPlaygroundExceededAtP2) {
    barriers_p2.unshift(LED_COUNT - barriers_p2.size() - 1);
    barriers_p1.pop();
    lastPressed = -1;
    speed = START_SPEED;
    light_up(CRGB::Green);
  }

  if(barriers_p1.size() == 0) {
    p1_win_stip();
    reset_game();
  }

  if(barriers_p2.size() == 0) {
    p2_win_stip();
    reset_game();
  }
}

unsigned long previousMillisStart = 0;

void start_blink() {
  FastLED.clear();
  
  unsigned long currentMillis = millis();
  unsigned long diff = currentMillis - previousMillisStart;

  if (diff >= 1000) {
    leds[15] = CRGB::Red;
    FastLED.show();
  }

  if(diff >= 2000) {
    leds[15] = CRGB::Black;
    FastLED.show();

    previousMillisStart = currentMillis;
  }
}

unsigned long previousMillisLeds = 0;

bool has_update() {
  unsigned long currentMillis = millis();
  unsigned long diff = currentMillis - previousMillisLeds;

  if(diff >= speed) {
    previousMillisLeds = currentMillis;
    return true;
  } else {
    return false;
  }
}

void update_leds() {
  FastLED.clear();

  int bp1_size = barriers_p1.size();
  int bp2_size = barriers_p2.size();

  for (int bp1 = 0; bp1 < bp1_size; bp1++) {
    leds[barriers_p1.get(bp1)] = CRGB::Green;
  }
  
  for (int bp2 = 0; bp2 < bp2_size; bp2++) {
    leds[barriers_p2.get(bp2)] = CRGB::Red;
  }

  for (int i = 0; i < LED_COUNT; i++) {    
    if (i == pos) {
      leds[i] = CRGB::Blue;
    }
  }
  
  FastLED.show();
}

bool pressedP1() {
  return digitalRead(PIN_BUTTON_P1) == HIGH;
}

bool pressedP2() {
  return digitalRead(PIN_BUTTON_P2) == HIGH;
}

int getAccelerationP1() {
  int bp1_size = barriers_p1.size();

  if(pos > bp1_size) {
    return -1;
  }

  return pos;
}

int getAccelerationP2() {
  int bp2_size = barriers_p2.size();

  if(pos < LED_COUNT - bp2_size - 1) {
    return -1;
  }

  return LED_COUNT - 1 - pos;
}

void loop() {

  if(pressedP1() && !game_started) {
    game_started = true;
    direction = 2;
  }

  if(pressedP2() && !game_started) {
    game_started = true;
    direction = 1;
  }

  if(!game_started) {
    start_blink();
    return;
  }

  int bp1_size = barriers_p1.size();
  int bp2_size = barriers_p2.size();

  if(has_update()) {
    move();
    update_leds();
  }

  bool isInBarrierP1 = pos < bp1_size;
  bool isInBarrierP2 = pos > LED_COUNT - bp2_size - 1;

  if(pos == 15) {
    lastPressed = -1;
  }

  if(pressedP1() && isInBarrierP1 && lastPressed != 2) {
    lastPressed = 2;
    speed = speeds[getAccelerationP1()];
    direction = 2;

    Serial.print("\nacc:");
    Serial.print(getAccelerationP1());
    Serial.print("\nspeed:");
    Serial.print(speed);
  }

  if(pressedP2() && isInBarrierP2 && lastPressed != 1) {
    lastPressed = 1;
    speed = speeds[getAccelerationP2()];
    direction = 1;
    Serial.print("\nacc:");
    Serial.print(getAccelerationP2());
    Serial.print("\nspeed:");
    Serial.print(speed);
  }
}