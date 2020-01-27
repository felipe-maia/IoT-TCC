void setup() {
  Serial.begin(9600);
  pinMode(14,INPUT);
}

void loop() {
  int sensorValue = analogRead(14);
  //Converter a leitura analógica (que vai de 0 - 4095 do esp32) para uma voltagem (0 - 3.3V), quanto de acordo com a intensidade de luz no LDR a voltagem diminui.
  float voltage = sensorValue * (3.3 / 4095.0);  
  Serial.println(voltage);
}
