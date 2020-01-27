int dataCompleta;
int ano = 2020;
int mes = 12;
int dia = 1;
char dataString[8];


void setup() {
    Serial.begin(9600);
}

void loop() {
  dataCompleta = ano*100;
  dataCompleta = dataCompleta +mes;
  dataCompleta = dataCompleta*100;
  dataCompleta = dataCompleta +dia;
  itoa(dataCompleta,dataString, 10); //int -> string
  //atoi(); string -> int
  Serial.println(dataString);
  delay(15000);

}
