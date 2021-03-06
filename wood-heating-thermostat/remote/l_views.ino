const char * rezimy_skr[] = {" Start", "Autom.", "Manual"};
void home(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Kotol Dom ");
  lcd.print( rezimy_skr[ rezim.get() ] );

  lcd.setCursor(0, 1); // new line

  align(lcd, teplota_kotol.get(), 3);
  lcd.print( (char) 223 );

  if (teplota_kotol.get() > teplota_kotol.get_prev()) {
    lcd.write(3);
  } else if (teplota_kotol.get() < teplota_kotol.get_prev()) {
    lcd.write(2);
  } else {
    lcd.print(" ");
  }

  align(lcd, teplota_dom.get(), 3);
  lcd.print( (char) 223 );

  if (rezim.get() == 1 || state) {
    align(lcd, teplota_ciel.get(), 6);
    lcd.print( (char) 223 );
  }

  if (state) {
    lcd.setCursor(14, 1);
    lcd.blink();
  } else {
    lcd.noBlink();
  }
}

void teplota_alarm_view(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Teplota alarm:");
  lcd.setCursor(0, 1); // new line

  align(lcd, teplota_alarm.get(), 15);
  lcd.print( (char) 223 );

  if (state) {
    lcd.setCursor(12, 1);
    lcd.blink();
  } else {
    lcd.noBlink();
  }
}

void alarm_view(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Alarm:");
  lcd.setCursor(0, 1); // new line
  
  if (alarm.get()) {
    lcd.print("         zapnuty");
  } else {
    lcd.print("         vypnuty");
  }

  if (state) {
    lcd.setCursor(9, 1);
    lcd.blink();
  } else {
    lcd.noBlink();
  }
}

void vlhkost_view(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Vlhkost vzduchu:");
  lcd.setCursor(0, 1); // new line
  
  align(lcd, dht_sensor.readHumidity(), 15);
  lcd.print( "%" );
}

void vykon_view(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Tah vzduchu:  ");
  align(lcd, vykon.get(), 2);
  lcd.setCursor(0, 1); // new line

  lcd.print("[");

  for (int i = 1; i < 15; ++i) {
    if (i <= vykon.get()) {
      lcd.write(1);
    } else {
      lcd.print(" ");
    }
  }
  
  lcd.print("]");

  if (state) {
    lcd.setCursor(vykon.get() + 1, 1);
    lcd.blink();
  } else {
    lcd.noBlink();
  }
}

const char * rezimy[] = {"           start", "     automaticky", "          manual"};
void rezim_view(LiquidCrystal_I2C& lcd, bool state) {
  lcd.print("Rezim:");
  lcd.setCursor(0, 1); // new line
  
  lcd.print( rezimy[ rezim.get() ] );

  if (state) {
    lcd.setCursor(4, 1);
    lcd.blink();
  } else {
    lcd.noBlink();
  }
}

void align(LiquidCrystal_I2C& lcd, int value, int width) {
  if (value == UNDEF) {
    for (int i = 0; i < width - 2; ++i) {
      lcd.print(' ');
    }
    lcd.print('-');
    if (width > 1) {
      lcd.print('-');
    }
    return;
  }
  if (value < 0) {
    width--;
  }

  if (value < 10 && value > -10) {
    width--;
  } else if (value > 99 || value < -99) {
    width -= 3;
  } else {
    width -= 2;
  }
  for (int i = 0; i < width; ++i) {
    lcd.print(' ');
  }

  lcd.print(value);
}

typedef struct {
  void (*fnc)(LiquidCrystal_I2C&, bool);
  Value<int8_t, origin, none> * data;
  const char * nazov;
} view;

view views[] = {
  {
    &home,
    &teplota_ciel,
  },
  {
    &vykon_view,
    &vykon,
  },
  {
    &alarm_view,
    &alarm,
  },
  {
    &teplota_alarm_view,
    &teplota_alarm
  },
  {
    &vlhkost_view,
    NULL,
  },
  {
    &rezim_view,
    &rezim
  }
};

const int view_count = sizeof(views) / sizeof(view);

Value<int> current_view(0, 0, view_count - 1);


Timer button_debounce(100, false, false);
Value<int> edit_state(0);

void view_edit_isr() {
  if (!button_debounce.started() && views[ current_view.get() ].data != NULL) {
    button_debounce.start();
    edit_state.set( !edit_state.get() );
    Serial.println(edit_state.get());
    enc_used.restart();

    if (beeping.started()) {
      alarm_snooze.start();
    }
  }
}

void render() {
  lcd.clear();
  //Serial.println( current_view.get() );
  views[ current_view.get() ].fnc(lcd, edit_state.get());
}

void store_data() {
  for (int i = 0; i < view_count; ++i) {
    int8_t value = views[ i ].data->get();
    EEPROM.update(i, value);
  }
}

void restore_data() {
  for (int i = 0; i < view_count; ++i) {
    int8_t value;
    EEPROM.get(i, value);
    if (views[ i ].data != NULL) {
      views[ i ].data->set( value, none );
    }
  }
}

