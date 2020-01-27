#define PIN_SENSOR_PIR 27
//#define PIN_BUZZER 23

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SENSOR_PIR, INPUT);
//  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  
  int sinal = digitalRead(PIN_SENSOR_PIR); 
  Serial.println(sinal);

  if(sinal == HIGH){
    //aciona o Buzzer
    Serial.println("ALARME ACIONADO");
    //digitalWrite(PIN_BUZZER, HIGH);
    delay(5000);
  }
}
