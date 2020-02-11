#include <SPI.h>
#include <WiFi.h>
#include <WiFiUDP.h> //Socket UDP
#include <NTPClient.h> //Biblioteca NTP para busca da data atual
#include <MFRC522.h> //Biblioteca RFID

#include <FirebaseESP32.h>
#include <FirebaseESP32HTTPClient.h>
#include <FirebaseJson.h>
#include <jsmn.h>

#define FIREBASE_HOST "https://segdepositobelico.firebaseio.com"
#define FIREBASE_AUTH "iGVRVXuQuFSPckJ3n7aaFrQszTS8vPmV081ZSosl"
#define FIREBASE_FCM_SERVER_KEY "AAAA9NpwjjU:APA91bFlVliyNhEw8sGs--uafV1yCuVBuJcBL_BJjbqmHWvzMt3P_gfgA4b_ovDGpAJOcYOXbQw5g7XyQXDlSvSURODGx04z04E57HPSGOtDNor30sjPBMXoNs6TJIXSYU7_av5J3K81"

#define WIFI_SSID "A3-128GB"
#define WIFI_PASSWORD "canismajoris"
#define PIN_SENSOR_PIR 36
#define PIN_BUZZER     15
#define SS_PIN         21
#define RST_PIN        22

FirebaseData firebaseData;
FirebaseJson jsonAcesso, jsonDisparo;

//variaveis para data
int timeZone = -3;
char* nomeDia[] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
struct Date{
    int diaDaSemana;
    int dia;
    int mes;
    int ano;
    int horas;
    int minutos;
    int segundos;
    int dataCompleta;
};

WiFiUDP udp; //Socket UDP que a lib utiliza para recuperar dados sobre o horário

NTPClient ntpClient( //Objeto responsável por recuperar dados sobre horário
    udp,                    //socket udp
    "0.br.pool.ntp.org",    //URL do server NTP
    timeZone*3600,          //Deslocamento do horário em relacão ao GMT 0
    60000);                 //Intervalo entre verificações online

MFRC522 mfrc522(SS_PIN, RST_PIN); //Instanciando a classe para leitura do RFID

//variaveis do sistema
boolean alarmeAtivado, alarmeAcionado;
String codigo;

void setup() {

  Serial.begin(9600);
  SPI.begin();     
  mfrc522.PCD_Init(); 

  pinMode(PIN_SENSOR_PIR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  //configuração acesso ao db firebase
  Firebase.begin (FIREBASE_HOST, FIREBASE_AUTH);
  //configuração do fireBaseMenssaging
  firebaseData.fcm.begin(FIREBASE_FCM_SERVER_KEY);

  Firebase.reconnectWiFi(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando WIFI");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Conectado: ");
  Serial.println(WiFi.localIP());

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
   Serial.println();

  inicializarVariaveis();
}

void loop() {
  leituraSensorPIR();
  leituraCartao();
}

void inicializarVariaveis(void){
  
  alarmeAtivado = true;
  alarmeAcionado = false;
  digitalWrite(PIN_BUZZER,LOW);

}

boolean buscaCartao(void){
  
//buscar dados firebase
  boolean busca = false;
  QueryFilter query;
  query.orderBy("codigoCartao");
  query.equalTo(codigo);

  if(Firebase.getJSON(firebaseData, "usuarios", query )) {
    if(firebaseData.jsonString() != "{}"){
       busca = true;
    }
  }else{
    Serial.println(firebaseData.errorReason());
  }
  query.clear();
  return busca;
}

void leituraCartao(void){
   codigo = "";
   if ( mfrc522.PICC_IsNewCardPresent()){
    
        if ( mfrc522.PICC_ReadCardSerial()){
          Serial.println();
          Serial.println("_____________________________________");
          Serial.println();

          for (byte i = 0; i < mfrc522.uid.size; i++) { 
           codigo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
           codigo.concat(String(mfrc522.uid.uidByte[i], HEX));
           }
           codigo.toUpperCase();
           if(buscaCartao()){
               if(alarmeAtivado){
                  if(alarmeAcionado){
                       Serial.println("Alarme Reativado");
                       acionaBuzzer(1);
                       alarmeAcionado = false;
                       montaJsonDisparo(formataData());
                       enviaDisparoFirebase();              
                  }else{
                       Serial.println("Alarme Desativado");
                       acionaBuzzer(2);
                       alarmeAtivado = false;
                       montaJsonAcesso(formataData());
                       enviaAcessoFirebase();
                  }
               }else{
                    Serial.println("Alarme Ativado");
                    acionaBuzzer(1);
                    alarmeAtivado = true;
                    alarmeAcionado = false;
                    montaJsonAcesso(formataData());
                    enviaAcessoFirebase();
               }
            
           }else{
                Serial.println("Cartão inválido");
                acionaBuzzer(3);             
           }
           Serial.println("Dados do Cartão RFID:");
           mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));
           mfrc522.PICC_HaltA();
        }
    }  
}

void leituraSensorPIR(void){
  
  if(alarmeAtivado && !alarmeAcionado ){
    bool sinal = (digitalRead(PIN_SENSOR_PIR));
    if(sinal == true){ //se houve movimento
       alarmeAcionado = true;
       Serial.println("Movimento Detectado");
       montaJsonDisparo(formataData());
       enviaDisparoFirebase();
       enviaNotificacao();
    }
  }
}

void enviaNotificacao(void){
  firebaseData.fcm.setPriority("high");
  firebaseData.fcm.setTimeToLive(1000000); //tempo de vida de aproximadamente 11 dias
  firebaseData.fcm.setTopic("DisparoAlarme");
  firebaseData.fcm.setNotifyMessage("Alarme Disparado", "Atenção, o alarme da reserva disparou!");
  if (Firebase.sendTopic(firebaseData)){
    //Success, print the result returned from server
    Serial.println(firebaseData.fcm.getSendResult());
  }else{
    //Failed, print the error reason
    Serial.println(firebaseData.errorReason());
  }
}

void enviaDisparoFirebase(void){
  if(Firebase.pushJSON(firebaseData,"/DisparoAlarme", jsonDisparo)) {
         Serial.println("Dados de DISPARO enviados com sucesso!");
  }else{
         Serial.println(firebaseData.errorReason());
  }  
}

void enviaAcessoFirebase(void){
  if(Firebase.pushJSON(firebaseData,"/Acessos", jsonAcesso)) {
         Serial.println("Dados de ACESSO enviados com sucesso!");
  }else{
         Serial.println(firebaseData.errorReason());
  }  
}

Date formataData(void){
  Date dataAtual = getDate();   
  dataAtual.dataCompleta = dataAtual.ano*100;
  dataAtual.dataCompleta = dataAtual.dataCompleta + dataAtual.mes;
  dataAtual.dataCompleta = dataAtual.dataCompleta*100;
  dataAtual.dataCompleta = dataAtual.dataCompleta + dataAtual.dia; 
  return dataAtual; 
}

void montaJsonDisparo(Date dataAtual){
  jsonDisparo.set("data",dataAtual.dataCompleta);
  jsonDisparo.set("hora",dataAtual.horas);
  jsonDisparo.set("min",dataAtual.minutos);
  jsonDisparo.set("seg",dataAtual.segundos);
  jsonDisparo.set("alarme",alarmeAcionado);
}

void montaJsonAcesso(Date dataAtual){
  jsonAcesso.set("data",dataAtual.dataCompleta);
  jsonAcesso.set("hora",dataAtual.horas);
  jsonAcesso.set("min",dataAtual.minutos);
  jsonAcesso.set("seg",dataAtual.segundos);
  jsonAcesso.set("statusAlarme",alarmeAtivado);
  jsonAcesso.set("codigoCartao",codigo);
}

void acionaBuzzer(int t){
  for(int i = 1; i<=t ;i++){
    digitalWrite(PIN_BUZZER,HIGH);
    delay(70);
    digitalWrite(PIN_BUZZER,LOW);
    delay(70);
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
