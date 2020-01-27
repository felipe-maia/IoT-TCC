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

void setup() {
  Serial.begin(9600);
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

  String busca = "DBC94C0A";
  //buscar dados firebase
  QueryFilter query;
  query.orderBy("codigoCartao");
  query.equalTo(busca);

  if(Firebase.getJSON(firebaseData, "usuarios", query )) {
    if(firebaseData.jsonString() == "{}"){
        Serial.println("Nao encontrou");
    }else{
      String resultado = firebaseData.jsonString();
      Serial.println(resultado);
    }
    
  }else{
    Serial.println(firebaseData.errorReason());
  }
  query.clear();

  delay(15000); 
}
