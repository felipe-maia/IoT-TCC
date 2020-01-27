#include <WiFi.h>
#include <FirebaseESP32.h>
#include <FirebaseESP32HTTPClient.h>
#include <FirebaseJson.h>
#include <jsmn.h>

#define FIREBASE_HOST "https://segdepositobelico.firebaseio.com"
#define FIREBASE_AUTH "iGVRVXuQuFSPckJ3n7aaFrQszTS8vPmV081ZSosl"
#define WIFI_SSID "A3-128GB"
#define WIFI_PASSWORD "canismajoris"

FirebaseData firebaseData;
FirebaseJson json;


void setup() {
  Serial.begin(9600);
  pinMode(14,INPUT);
  Firebase.begin (FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("conectado: ");
  Serial.println(WiFi.localIP());
}
void loop() {

//buscar dados firebase
  if(Firebase.getString(firebaseData,"Acessos/-Ly43SLsz35TB4Sw942t/codigoCartao" )) {
    if (firebaseData.dataType() == "string" ) {
      Serial.println(firebaseData.stringData());
    }
  }else{
    Serial.println(firebaseData.errorReason());
  }

  delay(15000);

  //enviar dados
  json.set("codigoCartao", "123B5T");
  json.set("dataHora", "10/10/2010");
  json.set("statusAlarme", "Ativo");

if (Firebase.pushJSON(firebaseData,"/Acessos", json)) {
  //Firebase.pushTimestamp(firebaseData,"/Acessos/testeTime");
  //firebaseData . dataPath ();
  //firebaseData . pushName ();
  //firebaseData . dataPath () + "/" + firebaseData . pushName ();
} else {
  Serial.println(firebaseData.errorReason());
}  
}
