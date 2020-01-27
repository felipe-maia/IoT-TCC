#include <SPI.h>
#include <WiFi.h>
#include <MFRC522.h>
#include <NTPClient.h> //Biblioteca NTPClient modificada
#include <WiFiUDP.h> //Socket UDP
#define SS_PIN 21
#define RST_PIN 22


const char* ssid     = "A3-128GB";
const char* password = "canismajoris";

WiFiServer server(80);

//Fuso Horário, no caso horário de verão de Brasília 
int timeZone = -3;
 
//Struct com os dados do dia e hora
struct Date{
    int diaDaSemana;
    int dia;
    int mes;
    int ano;
    int horas;
    int minutos;
    int segundos;
};
 
//Socket UDP que a lib utiliza para recuperar dados sobre o horário
WiFiUDP udp;
 
//Objeto responsável por recuperar dados sobre horário
NTPClient ntpClient(
    udp,                    //socket udp
    "0.br.pool.ntp.org",    //URL do server NTP
    timeZone*3600,          //Deslocamento do horário em relacão ao GMT 0
    60000);                 //Intervalo entre verificações online
 
//Nomes dos dias da semana
char* nomeDia[] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

MFRC522 mfrc522(SS_PIN, RST_PIN); //Instanciando a classe para leitura do RFID

void setup() {
   Serial.begin(9600);
   SPI.begin();     
   mfrc522.PCD_Init(); 
  
   pinMode(12, OUTPUT); 
   pinMode(32, OUTPUT);
   delay(10);
   
   // Começando a conexão
   Serial.print("Conectando em ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
       delay(500);
       Serial.print(".");
   }

   Serial.println("");
   Serial.println("WiFi conectada.");
   Serial.print("Endereço IP: ");
   Serial.println(WiFi.localIP());
   server.begin();

   //Inicializa o client NTP
   ntpClient.begin();
    
   //Espera pelo primeiro update online
   Serial.println("Esperando primeiro update da data");
   while(!ntpClient.update())
   {
       Serial.print(".");
       ntpClient.forceUpdate();
       delay(500);
   }
 
   Serial.println();
   Serial.println("Primeiro update completo");
   Serial.println("Leitor RFID pronto");
   Serial.println();
}
void loop() {

WiFiClient client = server.available();
String conteudo = "";
digitalWrite(12, LOW);
digitalWrite(32, LOW);

if ( mfrc522.PICC_IsNewCardPresent())
{
        if ( mfrc522.PICC_ReadCardSerial())
        {
           //Recupera os dados sobre a data e horário
           Date date = getDate();
                       
           for (byte i = 0; i < mfrc522.uid.size; i++) { 
           conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
           conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
           }
           conteudo.toUpperCase();
           if(conteudo.substring(1) == "C7 EF B5 26"){
                Serial.println("ACESSO AUTORIZADO");
                digitalWrite(12, HIGH);
                delay(1000);         
           }
           else{
                Serial.println("ACESSO NEGADO");
                digitalWrite(32, HIGH);
                delay(1000);
           }
           
           Serial.println("Dados da Tag RFID:");
           mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

           //Exibe a data/hora no Serial
           Serial.printf("%s - %02d/%02d/%d\n%02d:%02d:%02d",
            nomeDia[date.diaDaSemana],
            date.dia, 
            date.mes,
            date.ano, 
            date.horas,
            date.minutos, 
            date.segundos);
           Serial.println();
           Serial.println("--------------------------------");
           mfrc522.PICC_HaltA();
       }
}
}

Date getDate()
{
    //Recupera os dados de data e horário usando o client NTP
    char* strDate = (char*)ntpClient.getFormattedDate().c_str();
 
    //Passa os dados da string para a struct
    Date date;
    sscanf(strDate, "%d-%d-%dT%d:%d:%dZ", 
                    &date.ano, 
                    &date.mes, 
                    &date.dia, 
                    &date.horas, 
                    &date.minutos,
                    &date.segundos);
 
    //Dia da semana de 0 a 6, sendo 0 o domingo
    date.diaDaSemana = ntpClient.getDay();
    return date;
}
