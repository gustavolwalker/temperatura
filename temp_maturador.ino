#include <OneWire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

int releA = 3;
int releB = 2;
long delayB = 10000;
float tempA = 18;
float tempB = 19;
float tempDelay = 0.5;
int statusA = LOW;
int statusB = LOW;
float temp;
int led = 13;
int pinBotao = 0;
long valorBotao = 0;
boolean alterouTemp = false;


OneWire ds(10); 
LiquidCrystal lcd(12, 11, 8, 7, 6, 5);

void setup(void) {
  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(led, OUTPUT);     
  
  lcd.begin(16, 2);
  lcd.print("Temperatura!");
  lcd.clear();
  
  //tempA = EEPROM.read(releA);
  tempA = (float) (readFloat(0)-10);
  //tempB = EEPROM.read(releB);
  tempB = (float) (readFloat(4)-10);

  //Serial.begin(9600);
}

void loop(void) {
  digitalWrite(led, HIGH);  
  
  //Le botoes
  valorBotao = analogRead(pinBotao);
  //lcd.setCursor(12,0);
  //lcd.print(valorBotao,1);  

  if(valorBotao > 100 && valorBotao < 300){
    tempA = tempA + 0.5;
    alterouTemp=true;
  }else if(valorBotao > 301 && valorBotao < 400){
    tempA = tempA - 0.5;
    alterouTemp=true;
  }else if(valorBotao > 401 && valorBotao < 600){
    tempB = tempB + 0.5;
    alterouTemp=true;
  }else if(valorBotao >=601 && valorBotao < 1100){
    tempB = tempB - 0.5;
    alterouTemp=true;
  }
  
  //Processa temperatura
  temp = getTemp();
  
  float tempXA = tempA;
  if (statusA == HIGH){
    tempXA += tempDelay;
  }    
  float tempXB = tempB;
  if (statusB == HIGH){
    tempXB -= tempDelay;
  }  
  if (temp <= tempXA){
    statusA = HIGH;
  } else {
    statusA = LOW;
  }  
  if (temp >= tempXB){    
    if (delayB <= 0) {
      //Serial.println(99);
      statusB = HIGH;
    }
  } else {
    //Serial.println(98);
    if (statusB == HIGH) {
      //Serial.println(97);
      delayB = 240000;
    }
    statusB = LOW;    
  }    
  digitalWrite(releA, statusA);
  digitalWrite(releB, statusB);  

  //Imprime na serial
  /*Serial.print(tempXA);
  Serial.print(' ');
  Serial.print(statusA);
  Serial.print(' ');
  Serial.print(tempXB);
  Serial.print(' ');
  Serial.print(statusB);
  Serial.print(' ');
  Serial.print(temp);
  Serial.print(' ');
  Serial.println(delayB);*/

  //Imprime no display
  //lcd.noDisplay();
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.setCursor(6,0);
  lcd.print(temp,1);
  lcd.setCursor(0,1);
  lcd.print(statusA,1);
  lcd.print(statusB,1);
  lcd.print(" ");
  lcd.print(tempXA,1);
  lcd.print(" ");
  lcd.print(tempXB,1);
  lcd.print(" ");
  lcd.print((delayB/1000),1);
  //lcd.display();

  //Controle de delay para motor da geladeira
  if (delayB > 0) {
    delayB -= 250;
  }  
  
  //Grava temperatura na memoria
  if (alterouTemp){
    //EEPROM.write(releA, tempA);
    writeFloat(0, (tempA+10));
    //EEPROM.write(releB, tempB);
    writeFloat(4, (tempB+10));
    alterouTemp = false;
  }
  
  digitalWrite(led, LOW);
  delay(250);   
}

float getTemp(){
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
  ds.write(0x44,1); 

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


float readFloat(unsigned int addr) {
  union{
    byte b[4];
    float f;
  } data;
  for(int i = 0; i < 4; i++){
    data.b[i] = EEPROM.read(addr+i);
  }
  return data.f;
}

void writeFloat(unsigned int addr, float x){
  union {
    byte b[4];
    float f;
  } data;
  data.f = x;
  for(int i = 0; i < 4; i++) {
    EEPROM.write(addr+i, data.b[i]);
  }
}

