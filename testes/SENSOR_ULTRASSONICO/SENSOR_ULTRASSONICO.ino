//Sensor Ultrassônico HC-SR04
#include <Ultrasonic.h>

Ultrasonic ultrassom(16,17); //define o nome do sensor e os pinos trig(16) e echo(17)
int leitura;

void setup() {
Serial.begin(9600); //O monitor serial deverá estar em 9600 também para os dois se comunicarem
}

void loop()
{
leitura = ultrassom.Ranging(CM); //armazena a leitura na variável 
Serial.print(leitura); //imprime a leitura na tela
Serial.println("cm"); //coloca cm atrás da leitura
delay(10000);
}
