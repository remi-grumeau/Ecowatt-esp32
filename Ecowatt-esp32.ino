#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>


// WIFI
const char* _wifi_ssid  = "***";
const char* _wifi_pwd   = "****";

// A RECUPERER DEPUIS L INTERFACE RTE
// DANS L ONGLET "MES APPLICATIONS"
// ID Client | ID Secret ---> bouton "Copier en base 64"
const String _rte_key   = "";

const String _api_url   = "https://digital.iservices.rte-france.com/";
unsigned int _loop      = 0;

String _access_token    = "";

unsigned int _day1    = 0;
unsigned int _day2    = 0;
unsigned int _day3    = 0;
unsigned int _day4    = 0;


// CREATE WIFI CLIENT TO REQUEST API
WiFiClient _client;
HTTPClient _http;

void setup() {

  // START
  Serial.begin(115200);
  Serial.println("***************");
  Serial.println("DEMARRAGE");

  // CONNECT TO WIFI
  Serial.print("Connexion au WIFI ");
  Serial.println(_wifi_ssid);
  delay(200);

  // MAKE SURE WIFI IS CLEAR
//  WiFi.disconnect(1,1);
//  WiFi.mode(WIFI_STA);
  WiFi.begin(_wifi_ssid, _wifi_pwd);

  // WAIT FOR WIFI TO CONNECT
  while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      Serial.print(".");
  }

  Serial.println("connecté !");
  Serial.println("");
  Serial.print("adresse IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("adresse MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println("");
}


void getAccessToken()
{
  Serial.println("Recupération du token OAuth RTE");

  String _endpoint = _api_url;
  _endpoint += "token/oauth/";
    
  _http.begin(_endpoint);
  _http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  _http.addHeader("Authorization", "Basic "+_rte_key);
  int _http_response_code = _http.GET();
  if(_http_response_code>0)
  {
    // CHECK CALLBACK STRING
    String _http_response = _http.getString();

    // PARSE JSON CALLBACK STRING
    JSONVar auth_request = JSON.parse(_http_response);

    // IF PARSING GOES WRONG
    if (JSON.typeof(auth_request) == "undefined") {
      Serial.println("Erreur de parsing du JSON");
      return;
    }

    // IF PARSING GOES WRONG
    if(auth_request["access_token"]!=""){
      _access_token = auth_request["access_token"];
      Serial.println(_access_token);
    }
  }
  _http.end();
}



void getEcowatt()
{
  Serial.println("Recupération des datas EcoWatt");

  String _endpoint = _api_url;
  _endpoint += "open_api/ecowatt/v4/sandbox/signals";
    
  _http.begin(_endpoint);
  _http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  _http.addHeader("Authorization", "Bearer "+_access_token);
  int _http_response_code = _http.GET();
  if(_http_response_code>0)
  {
    // CHECK CALLBACK STRING
    String _http_response = _http.getString();
    Serial.println(_http_response);

    // PARSE JSON CALLBACK STRING
    JSONVar _ecowatt_data = JSON.parse(_http_response);

    // IF PARSING GOES WRONG
    if (JSON.typeof(_ecowatt_data) == "undefined") {
      Serial.println("Erreur de parsing du JSON");
      return;
    }

    // IF PARSING GOES WRONG
    if(_ecowatt_data["signals"]!="")
    {
//      POUR VOIR LE JEU DE DONNEES PAR JOUR
//      Serial.println(_ecowatt_data["signals"][0]);

      _day1 = int(_ecowatt_data["signals"][0]["dvalue"]);
      _day2 = int(_ecowatt_data["signals"][1]["dvalue"]);
      _day3 = int(_ecowatt_data["signals"][2]["dvalue"]);
      _day4 = int(_ecowatt_data["signals"][3]["dvalue"]);

    }
  }
  _http.end();
}



void loop() {


  // RECUPERATION DU TOKEN POUR ACCEDER AUX API RTE
  if(_access_token=="")
  {
    getAccessToken();
    delay(500);
  }

  // RECUPERATION DES DONNEES DEPUIS L API ECOWATT
  if(_loop==0){
    getEcowatt();
    delay(500);
  }

  _loop++;


  // LA VALEUR VA DE
  // 1 - VERT
  // 2 - ORANGE
  // 3 - ROUGE
  Serial.print("Etat pour les 4 prochains jours : ");
  Serial.print(_day1);
  Serial.print(" ");
  Serial.print(_day2);
  Serial.print(" ");
  Serial.print(_day3);
  Serial.print(" ");
  Serial.print(_day4);
  Serial.println("");

  // UNE MINUTE D ATTENTE
  delay(60000);

  // RECHARGER LES DONNEES TOUTES LES 20 MIN
  // POUR NE PAS DEPASSER LE PLAFOND DE REQUETES AUTORISEES PAR JOUR
  if(_loop==20)
    _loop==0;
 
}
