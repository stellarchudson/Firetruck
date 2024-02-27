/// @file    FiretruckCode_ver5.ino
/// @brief   Current code for the dashboard light control in the firetruck- Includes button control, added sound trigger integration
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.


//TODO:
//fix flash on color


#include <FastLED.h>
#include <ezButton.h>
#include <pixeltypes.h>
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

#define LED_PIN 7
#define keyOut 3
#define out1 9
#define out2 10
#define out3 11
#define out4 12
#define NUM_LEDS 300
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER BRG
#define UPDATES_PER_SECOND 100

#define DEFAULT_I2C_ADDR 0x3A

//THESE ARE ARCADE PINS
#define SWITCH1 18  // PA01
#define SWITCH2 19  // PA02
#define SWITCH3 20  // PA03
#define SWITCH4 2   // PA06
#define PWM1 12     // PC00
#define PWM2 13     // PC01
#define PWM3 0      // PA04
#define PWM4 1      // PA05

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType currentBlending;
Adafruit_seesaw ss;

ezButton keyIgnit(2);
ezButton colorChange(4);  //changed for bad pin holder
ezButton switchOne(5);
ezButton switchTwo(6);


const char* palList[2] = { "LavaColors_p", "OceanColors_p" };
bool keyOn = false;
bool notButt = true;
int n;


void setup() {
  n = 1;
  delay(3000);  // power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  currentBlending = LINEARBLEND;
  FastLED.setTemperature(Candle);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(9600);

  keyIgnit.setDebounceTime(50);
  colorChange.setDebounceTime(50);
  switchOne.setDebounceTime(50);
  switchTwo.setDebounceTime(50);

  while (!Serial) delay(10);  // wait until serial port is opened

  Serial.println(F("Adafruit PID 5296 I2C QT 4x LED Arcade Buttons test!"));

  if (!ss.begin(DEFAULT_I2C_ADDR)) {
    Serial.println(F("seesaw not found!"));
    while (1) delay(10);
  }

  uint16_t pid;
  uint8_t year, mon, day;

  ss.getProdDatecode(&pid, &year, &mon, &day);
  Serial.print("seesaw found PID: ");
  Serial.print(pid);
  Serial.print(" datecode: ");
  Serial.print(2000 + year);
  Serial.print("/");
  Serial.print(mon);
  Serial.print("/");
  Serial.println(day);

  if (pid != 5296) {
    Serial.println(F("Wrong seesaw PID"));
    while (1) delay(10);
  }

  Serial.println(F("seesaw started OK!"));
  //these are seesaw pins
  ss.pinMode(SWITCH1, INPUT_PULLUP);
  ss.pinMode(SWITCH2, INPUT_PULLUP);
  ss.pinMode(SWITCH3, INPUT_PULLUP);
  ss.pinMode(SWITCH4, INPUT_PULLUP);
  ss.analogWrite(PWM1, 127);
  ss.analogWrite(PWM2, 127);
  ss.analogWrite(PWM3, 127);
  ss.analogWrite(PWM4, 127);

  //these are soundboard output pins
  pinMode(keyOut, OUTPUT);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out4, OUTPUT);
}

uint8_t incr = 0;



void loop() {

  arcadeButts();


  //begin buttons
  keyIgnit.loop();
  colorChange.loop();
  switchOne.loop();
  switchTwo.loop();

  //LED control
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FastLED.delay(1000 / UPDATES_PER_SECOND);
  FillLEDsFromPaletteColors(startIndex);
  pacifica_deepen_colors();
  FastLED.show();

  defaultMode();
  buttonMode();

  if (keyOn) {

    if (notButt) {
      FastLED.setTemperature(Candle);
      fill_solid(currentPalette, 16, CRGB::Linen);
      flashMode(1);
    }

    if (switchOne.getState() == HIGH) {
      notButt = true;
      currentBlending = NOBLEND;
      currentPalette = RainbowColors_p;
      flashMode(2);
      buttonMode();  //remove this if you don't want the buttons to change rainbow tone
    }

    if (switchOne.isPressed() && notButt) {
      FastLED.setTemperature(Candle);
      fill_solid(currentPalette, 16, CRGB::Linen);
      flashMode(1);
    }
  }
}




void buttonMode() {
  currentBlending = LINEARBLEND;
  if (colorChange.isPressed()) {
    notButt = false;
    if (n == 0) {
      FastLED.setTemperature(Candle);
      currentPalette = LavaColors_p;
      flashMode(3);
    } else if (n == 1) {
      FastLED.setTemperature(OvercastSky);
      currentPalette = OceanColors_p;
      flashMode(4);
    }
    n++;
    if (n == 2) { n = 0; }  //make sure n resets
  }
}


bool defaultMode() {
  //"default" mode is the pale pink- basically just means rainbow isn't overwriting everything else
  if (keyIgnit.isPressed()) {

    if (!keyOn) {
      fill_solid(currentPalette, 16, CRGB::Linen);
      keyOn = true;
      digitalWrite(keyOut, HIGH);
      delay(200);
      digitalWrite(keyOut, LOW);
    }
  }

  if (keyIgnit.isReleased()) {
    keyOn = false;
    uint8_t brightness = 0;
    fill_solid(currentPalette, 16, CRGB::Black);
    FastLED.clear();
    FastLED.show();
    //if I have some extra time/pins, would be nice to add a de-rev sound
  }
  return keyOn;
}

void flashMode(int resetPal) {
  switch (resetPal) {
    case 1:
      fill_solid(currentPalette, 16, CRGB::Linen);
      if (switchTwo.getState() == HIGH) {
        //FastLED.clear();
        currentPalette[0] = CRGB::Black;
        currentPalette[4] = CRGB::Black;
        currentPalette[8] = CRGB::Black;
        currentPalette[12] = CRGB::Black;
      }

      break;

    case 2:
      currentPalette = RainbowColors_p;
      if (switchTwo.getState() == HIGH) {
        //FastLED.clear();
        currentPalette[0] = CRGB::Black;
        currentPalette[4] = CRGB::Black;
        currentPalette[8] = CRGB::Black;
        currentPalette[12] = CRGB::Black;
      }

      break;

    case 3:
      currentPalette = LavaColors_p;
      if (switchTwo.getState() == HIGH) {
        //FastLED.clear();
        currentPalette[0] = CRGB::Black;
        currentPalette[4] = CRGB::Black;
        currentPalette[8] = CRGB::Black;
        currentPalette[12] = CRGB::Black;
      }
      break;

    case 4:
      currentPalette = OceanColors_p;
      if (switchTwo.getState() == HIGH) {
        //FastLED.clear();
        currentPalette[0] = CRGB::Black;
        currentPalette[4] = CRGB::Black;
        currentPalette[8] = CRGB::Black;
        currentPalette[12] = CRGB::Black;
      }
      break;

    default:
      break;
  }
  // }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex) {
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}


// Deepen the blues and greens
void pacifica_deepen_colors() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8(leds[i].blue, 145);
    leds[i].green = scale8(leds[i].green, 200);
    leds[i] |= CRGB(2, 5, 7);
  }
}

void arcadeButts() {
  if (!ss.digitalRead(SWITCH1)) {
    ss.analogWrite(PWM1, incr);
    incr += 5;
    digitalWrite(out1, HIGH);
    delay(200);
    digitalWrite(out1, LOW);
  } else {
    ss.analogWrite(PWM1, 0);
  }

  if (!ss.digitalRead(SWITCH2)) {
    ss.analogWrite(PWM2, incr);
    incr += 5;
    digitalWrite(out2, HIGH);
    delay(200);
    digitalWrite(out2, LOW);
  } else {
    ss.analogWrite(PWM2, 0);
  }

  if (!ss.digitalRead(SWITCH3)) {
    ss.analogWrite(PWM3, incr);
    incr += 5;
    digitalWrite(out3, HIGH);
    delay(200);
    digitalWrite(out3, LOW);
  } else {
    ss.analogWrite(PWM3, 0);
  }

  if (!ss.digitalRead(SWITCH4)) {
    ss.analogWrite(PWM4, incr);
    incr += 5;
    digitalWrite(out4, HIGH);
    delay(200);
    digitalWrite(out4, LOW);
  } else {
    ss.analogWrite(PWM4, 0);
  }
  delay(10);
}
