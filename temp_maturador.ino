/*
 Temperatura
 Is a temperature controller with two relay to control Heating and Cooling.Those relay are photocoupler;

 The thermostat use 3 wire, two to power and one to signal. This use port digital 9;
 The relay A use port digital 11.
 The relay B use port digital 12;
 The keyboard controls the raising and lowering of the temperature, and use port analog 0
 */
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

//Variaveis reles
int releA = 12;
int releB = 11;
long delayB = 10000;
float tempA = 18;
float tempB = 19;
float tempSetA = 18;
float tempSetB = 19;
float tempDelay = 0.5;
int statusA = LOW;
int statusB = LOW;
//Variaveis temperatura
int termometro = 9;
float temp;
//Variaveis keyboard
int keyboard = 0;
int valorKey = 0;
int valorSetKey = 0;
int valorSetDelay = 2000;
boolean alterouTemp = false;
//Variais serial
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
//Variaveis diversas
int led = 13;
long delayStart = 1500;
int delaySystem = 250;
//Inicializacao bibliotecas
OneWire ds(termometro);
LiquidCrystal lcd(4, 3, 8, 7, 6, 5);

void setup(void) {
  lcd.begin(16, 2);
  lcd.print("Temperatura!");
  lcd.setCursor(0, 1);
  lcd.print(" Aguarde... ");

  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(termometro, INPUT);
  pinMode(led, OUTPUT);

  Serial.begin(9600);
  inputString.reserve(200);

  //Debug fixa valor de A e B
  //writeFloat(2, (10));
  //writeFloat(6, (11));
  //releA
  tempA = (float) (readFloat(2) - 10);
  tempSetA = tempA;
  //releB
  tempB = (float) (readFloat(6) - 10);
  tempSetB = tempB;
}

void loop(void) {
  digitalWrite(led, HIGH);

  if (delayStart <= 0) {
    //Le botoes
    valorKey = analogRead(keyboard);
    if (stringComplete) {
      Serial.print(inputString);
      valorKey = inputString.toInt();
      // clear the string:
      inputString = "";
      stringComplete = false;
    }

    //Caso prescionou uma tecla reinicia contagem
    if (valorKey < 800) {
      valorSetDelay = 2000;
    }

    //Mantenha prescionado por 2 segundos para entrar em SET
    if (valorKey < 10 && valorSetKey < 2000) {
      valorSetKey += delaySystem;
    }
    //Testa botoes
    if (valorSetKey >= 2000) {
      if (valorKey > 700 && valorKey < 800) {
        tempSetB = tempSetB + 0.5;
        alterouTemp = true;
      } else if (valorKey > 400 && valorKey < 500) {
        tempSetB = tempSetB - 0.5;
        alterouTemp = true;
      } else if (valorKey > 300 && valorKey < 400) {
        tempSetA = tempSetA + 0.5;
        alterouTemp = true;
      } else if (valorKey > 100 && valorKey < 200) {
        tempSetA = tempSetA - 0.5;
        alterouTemp = true;
      } else if (valorKey < 10 && alterouTemp) {
        valorSetKey = 0;
      }
    }

    //Processa temperatura
    temp = getTemp();
    float tempXA = tempA;
    if (statusA == HIGH) {
      tempXA += tempDelay;
    }
    float tempXB = tempB;
    if (statusB == HIGH) {
      tempXB -= tempDelay;
    }
    if (temp <= tempXA) {
      statusA = HIGH;
    } else {
      statusA = LOW;
    }
    if (temp >= tempXB) {
      if (delayB <= 0) {
        statusB = HIGH;
      }
    } else {
      if (statusB == HIGH) {
        delayB = 240000;
      }
      statusB = LOW;
    }
    digitalWrite(releA, statusA);
    digitalWrite(releB, statusB);

    //Imprime na serial
    /*if (valorSetKey >= 2000) {
      printSerial(tempSetA, tempSetB);
    } else {
      printSerial(tempXA, tempXB);
    }*/
    
    //Imprime no display
    if (valorSetKey >= 2000) {
      printDisplay(tempSetA, tempSetB);
    } else {
      printDisplay(tempXA, tempXB);
    }

    //Controle de delay para motor da geladeira
    if (delayB > 0) {
      delayB -= delaySystem;
    }

    //Grava temperatura na memoria
    if (alterouTemp && valorSetKey == 0) {
      //releA
      tempA = tempSetA;
      writeFloat(2, (tempA + 10));
      //releB
      tempB = tempSetB;
      writeFloat(6, (tempB + 10));
      alterouTemp = false;
      valorSetKey = 0;
    }

    //Caso nao teclar por 2 segundos desetiva SET
    if (valorSetDelay == 0) {
      alterouTemp = false;
      valorSetKey = 0;
      tempSetA = tempA;
      tempSetB = tempB;
    } else {
      valorSetDelay -= delaySystem;
    }
  } else {
    delayStart -= delaySystem;
    if (delayStart <= 0) {
      getTemp();
      lcd.clear();
    }
  }
  digitalWrite(led, LOW);
  delay(delaySystem);
}

//Funcoes do sistema
float getTemp() {
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    //Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    //Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  for (int i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float TRead = ((MSB << 8) | LSB);
  float Temperature = TRead / 16;

  return Temperature;
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n'  || inChar == '\r\n') {
      stringComplete = true;
    }
  }
}

float readFloat(unsigned int addr) {
  union {
    byte b[4];
    float f;
  }
  data;
  for (int i = 0; i < 4; i++) {
    data.b[i] = EEPROM.read(addr + i);
  }
  return data.f;
}

void writeFloat(unsigned int addr, float x) {
  union {
    byte b[4];
    float f;
  }
  data;
  data.f = x;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addr + i, data.b[i]);
  }
}

void printSerial(float tempA , float tempB) {
  Serial.print(tempA);
  Serial.print(' ');
  Serial.print(statusA);
  Serial.print(' ');
  Serial.print(tempB);
  Serial.print(' ');
  Serial.print(statusB);
  Serial.print(' ');
  Serial.print(temp);
  Serial.print(' ');
  Serial.println(delayB);
}

void printDisplay(float tempA , float tempB) {
  lcd.setCursor(0, 0);
  lcd.print("Temp:        ");
  lcd.setCursor(6, 0);
  lcd.print(temp, 1);
  lcd.setCursor(12, 0);
  lcd.print("    ");
  lcd.setCursor(13, 0);
  if (valorSetKey >= 2000 ) {
    lcd.print("SET");
  }
  lcd.setCursor(0, 1);
  lcd.print(statusA, 1);
  lcd.print(statusB, 1);
  lcd.print(" ");
  lcd.print(tempA, 1);
  lcd.print(" ");
  lcd.print(tempB, 1);
  lcd.print(" ");
  lcd.print((delayB / 1000), 1);
}
