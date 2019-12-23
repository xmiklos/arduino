
#include <LiquidCrystal_I2C.h>
#include <RotaryEncoder.h>
#include <DallasTemperature.h>
#include "RF24.h"
#include "printf.h"
#include <EEPROM.h>

#include <Timer.h>
#include <Value.h>

#define PIN_DALLAS_TEMP     17  // A3

#define PIN_CURRENT_SENSOR  16  // A2
#define PIN_CURRENT_ENABLE  15  // A1

#define PIN_RF24_CE         9
#define PIN_RF24_CS         10
#define PIN_RF24_INT        2

#define PIN_ENCODER_SW      3
#define PIN_ENCODER_A       4
#define PIN_ENCODER_B       5

#define PIN_ACTUATOR_B      6
#define PIN_ACTUATOR_A      7

#define LCD_ADDRESS         0x27

#define UNDEF -127

enum origin {
  none = 0,
  user,
  network
};

Value<int8_t> teplota_kotol((int8_t) UNDEF);
Value<int8_t> teplota_dom((int8_t) UNDEF);
Value<int8_t, origin, none> zatvor_vzduch((int8_t) 45);
Value<int8_t, origin, none> teplota_ciel((int8_t) 20);
Value<int8_t, origin, none> vykon((int8_t) 0, 0, 13);
Value<int8_t, origin, none> rezim((int8_t) 0, 0, 2);

RotaryEncoder enc(PIN_ENCODER_A, PIN_ENCODER_B);
Timer enc_used(20000, false, false);

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
uint8_t arrow_down_char[8]  = {  0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04, 0x00 };
uint8_t arrow_up_char[8]    = {  0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00 };


uint8_t full_char[8] = {  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

OneWire temp_wire(PIN_DALLAS_TEMP);
DallasTemperature dallas_temp(&temp_wire);

RF24 radio(PIN_RF24_CE, PIN_RF24_CS);
byte address[][5] = { 0xCC,0xCE,0xCC,0xCE,0xCC , 0xCE,0xCC,0xCE,0xCC,0xCE };

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here
{
  enc.tick();
  enc_used.restart();
}
