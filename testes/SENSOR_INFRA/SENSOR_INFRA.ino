#include <SPI.h>

void setup() {
  SPI.begin();
  Serial.begin(9600);
  pinMode(14, INPUT); // Define o pino do sensor como entrada
}

void loop() {
  bool sensor = digitalRead(14);
  if(sensor){
    Serial.println("movimento detectado");
    delay(2000);
  }
}
