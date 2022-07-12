#define _LCD_TYPE 2
#define BTN_UP   1
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_SELECT 5
#define BTN_NONE 10

#include <EEPROM.h>
#include <LCD_1602_RUS_ALL.h>
#include <TroykaDHT.h>
DHT dht(1, DHT22);

LCD_1602_RUS lcd(8, 9, 4, 5, 6, 7 );//For LCD Keypad Shield


uint32_t tmr1;
uint32_t tmr2;
uint32_t tmr3;
uint32_t tmr4;

boolean strab = false;

byte light = 10;
byte relay = 0;
float setTemp;
float delta;
byte mode = 0;
byte pointer = 0;

int detectButton() {
  int keyAnalog =  analogRead(A0);
  if (keyAnalog < 100) {
    // Значение меньше 100 – нажата кнопка right
    return BTN_RIGHT;
  } else if (keyAnalog < 200) {
    // Значение больше 100 (иначе мы бы вошли в предыдущий блок результата сравнения, но меньше 200 – нажата кнопка UP
    return BTN_UP;
  } else if (keyAnalog < 400) {
    // Значение больше 200, но меньше 400 – нажата кнопка DOWN
    return BTN_DOWN;
  } else if (keyAnalog < 600) {
    // Значение больше 400, но меньше 600 – нажата кнопка LEFT
    return BTN_LEFT;
  } else if (keyAnalog < 800) {
    // Значение больше 600, но меньше 800 – нажата кнопка SELECT
    return BTN_SELECT;
  } else {
    // Все остальные значения (до 1023) будут означать, что нажатий не было
    return BTN_NONE;
  }
}

void setup() {
  EEPROM.get(0, setTemp);
  EEPROM.get(4, delta);
  pinMode(light, OUTPUT);
  digitalWrite(light, 1);
  pinMode(relay, OUTPUT);

  //  Записать данные из памяти по температуре и дельте
  
  dht.begin();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("   Контроллер");
  lcd.setCursor(0, 1);
  lcd.print(" отопления v2.0");
  delay(1000);
  lcd.clear();
  
}

void loop() {
  int button = detectButton();
  
  if (millis() - tmr1 >= 10000) {  // ищем разницу
    digitalWrite(10, LOW);
  } else {
    digitalWrite(10, 1);
  }

  //режимы работы 0-текущий режим 1-режим настроек
  switch (mode) {
    case 0: {
      if (millis() - tmr3 >= 200) { 
        tmr3 = millis();
        //Действия на нажатие кнопок
        if (button == BTN_SELECT) {
          tmr1 = millis();
          mode = 1;
        }
        if (button != BTN_NONE){
          tmr1 = millis();
        }
      }
      //Проверка температуры раз в 1 сек.
      if (millis() - tmr2 >= 1000) {  // ищем разницу
        tmr2 = millis();
        dht.read();
        if (dht.getState() == DHT_OK) {
          if (dht.getTemperatureC() >= setTemp) {
            digitalWrite(relay, LOW);
          }
          if (dht.getTemperatureC() <= setTemp - delta) {
            digitalWrite(relay, HIGH);
          }
          lcd.setCursor(7, 0);
          lcd.print("  ");
          lcd.setCursor(0, 0);
          lcd.print("T=");
          lcd.setCursor(2, 0);
          lcd.print(String(dht.getTemperatureC()));
          lcd.setCursor(9, 0);
          lcd.print("V=");
          lcd.setCursor(11, 0);
          lcd.print(String(dht.getHumidity()));
          lcd.setCursor(0, 1);
          lcd.print("T=");
          lcd.setCursor(2, 1);
          lcd.print(String(setTemp));
          lcd.setCursor(9, 1);
          lcd.print("D=");
          lcd.setCursor(11, 1);
          lcd.print(String(delta));

          //Отображение символа работы насоса
          if (digitalRead(relay) == HIGH) {
            lcd.setCursor(15, 1);
            lcd.print("+");
          } else {
            lcd.setCursor(15, 1);
            lcd.print("-");
          }
        }
      } 
      break;  
    }
    case 1: {
      lcd.setCursor(0, 0);
      lcd.print("   Настройка    ");
      if (millis() - tmr3 >= 200) { 
        tmr3 = millis();
        switch (button) {
        case BTN_UP: {
          tmr1 = millis();
          if (pointer == 0) {
            setTemp += 0.1;
          }
          if (pointer == 1) {
            delta += 0.1;
          }
          break;
        }
        case BTN_DOWN: {
          tmr1 = millis();
          if (pointer == 0) {
            setTemp -= 0.1;
          }
          if (pointer == 1) {
            delta -= 0.1;
          }
          break;
        }
        case BTN_LEFT: {
          tmr1 = millis();
          if (pointer > 0) {
            pointer -= 1;
          }
          break;
        }
        case BTN_RIGHT: {
          if (pointer < 1) {
            pointer += 1;
          }
          tmr1 = millis();
          break;
        }
        case BTN_SELECT: {
          EEPROM.put(0, setTemp);
          EEPROM.put(4, delta);
          mode = 0;
          tmr1 = millis();
          pointer = 0;
          break;
        }
      }
      }

      if (millis() - tmr4 >= 500) {
        tmr4 = millis();
        strab = !strab;
      }

      
      if (pointer == 0) {
        if (strab == true) {
          lcd.setCursor(0, 1);
          lcd.print("T=");
          lcd.setCursor(2, 1);
          lcd.print(String(setTemp));
        } else {
          lcd.setCursor(0, 1);
          lcd.print("        ");
        }
        lcd.setCursor(9, 1);
        lcd.print("D=");
        lcd.setCursor(11, 1);
        lcd.print(String(delta));
      }
      
      if (pointer == 1) {
        if (strab == true) {
          lcd.setCursor(9, 1);
          lcd.print("D=");
          lcd.setCursor(11, 1);
          lcd.print(String(delta));
        } else {
          lcd.setCursor(9, 1);
          lcd.print("        ");
        }
        lcd.setCursor(0, 1);
        lcd.print("T=");
        lcd.setCursor(2, 1);
        lcd.print(String(setTemp));
      }
      break;
    }
  }
}
