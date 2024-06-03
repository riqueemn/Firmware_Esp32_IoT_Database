//Includes
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <math.h>
#include <LiquidCrystal.h>

//Defines Pinout pins
#define LEITURA_ADC   36    // GPIO36
#define ENABLE_ADC    14    // GPIO14
#define CHAVE         27    // GPIO27
LiquidCrystal lcd    (18,   //RS no IO17, 
                      19,   //EN no IO19
                      16,   //D4 no IO16
                      17,   //D5 no IO17
                      25,   //D6 no IO25
                      26);  //D7 no IO26

struct Sensors { // This structure is named "myDataType"
  float   intensidade_UV = 0;
  float intensidade_UV_Anterior = 0;
  float   intensidade_UV_Display = 0;
  float intensidade_UV_Display_Anterior = 0;
};

//variaveis Globais
float   valor_AD = 0;
int     estado_chave ;
float   tensao_AD ;

Sensors Sensores;


//Constants
HTTPClient  http;
String      URL = "http://192.168.1.116/Server_DataBase_Esp32/server.php";
const char* ssid = "LESC"; 
const char* password = "A33669608F"; 

//Funções;
void  connectWiFi() {
  int i = 0;
  int k = 1;
  bool b = false;

  WiFi.mode(WIFI_OFF);
  delay(1000);
  //This line hides the viewing of ESP as wifi hotspot
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("Conectando Wifi");

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Conectando Wifi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if(i >= 15 && k < 3){
      i = 0;
      k++;
      
    } else if (i >= 15 && k >= 3){
      b = !b;
      i = 0;
      k = 1;
      
    }

    if(!b){
      lcd.setCursor(i, k);
      lcd.print(".");
    } else {
      lcd.setCursor(i, k);
      lcd.print(" ");
    }
    
    i++;
  }
    
  Serial.print("connected to : "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  lcd.clear();
}
float media_AD(float pinodeleitura)
{
  byte numeroleituras = 20;
  float media = 0; 
  for(int x = 0 ; x < numeroleituras ; x++)
    media += analogRead(pinodeleitura);
  media /= numeroleituras;
  return(media);  
}
void  medicao_uv(){
  estado_chave = digitalRead(CHAVE);
  
    if (estado_chave == 0)
    {
      digitalWrite(ENABLE_ADC, HIGH);
      valor_AD = media_AD(LEITURA_ADC);
      tensao_AD = (valor_AD*3.3)/4095;
      Sensores.intensidade_UV = (((tensao_AD - 1)*15)/2);
      Serial.println("ADC = " + String(valor_AD));
      Serial.println("Tensão = " + String(tensao_AD) + "V");
    if (Sensores.intensidade_UV <0)
    {
      Sensores.intensidade_UV = 0;
      Serial.println("UV = " + String(Sensores.intensidade_UV) + "mW/cm²");
      Serial.println("");
    }
    else{
      Serial.println("UV = " + String(Sensores.intensidade_UV) + "mW/cm²");
      Serial.println("");
    }
    
    }

    else{
      digitalWrite(ENABLE_ADC, LOW);
    }

    Sensores.intensidade_UV_Display = Sensores.intensidade_UV;

    delay(1000);
}

void writeDisplay(){
  if(Sensores.intensidade_UV_Display != Sensores.intensidade_UV_Display_Anterior){

      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Intensidade UV");
      lcd.setCursor(2, 1);
      lcd.print(Sensores.intensidade_UV);
      lcd.print(" mW/cm^2");

      Sensores.intensidade_UV_Display_Anterior = Sensores.intensidade_UV_Display;
  }
}

void  setup() {
  Serial.begin(9600);

  pinMode(LEITURA_ADC, INPUT);
  pinMode(ENABLE_ADC, OUTPUT);
  pinMode(CHAVE, INPUT_PULLUP);
  lcd.begin(16, 4);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Intensidade UV");

  connectWiFi();
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
}
void  wifi_loop() {
  if(WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if(Sensores.intensidade_UV != Sensores.intensidade_UV_Anterior){
    String postData = "uv_intensidade=" + String(Sensores.intensidade_UV)/*+"&temperatura=" +String(temperatura)*/;
    int httpCode = http.POST(postData);
    String payload = http.getString();

    if(httpCode > 0) {
      // file found at server
      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      } else {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    //http.end();  //Close connection

    Serial.print("URL : "); Serial.println(URL); 
    Serial.print("Data: "); Serial.println(postData);
    Serial.print("httpCode: "); Serial.println(httpCode);
    Serial.print("payload : "); Serial.println(payload);
    Serial.println("--------------------------------------------------");
    delay(1000);

    Sensores.intensidade_UV_Anterior = Sensores.intensidade_UV;
  }
}


//Loop Principal
void loop() {
  medicao_uv();
  writeDisplay();
  wifi_loop();
}