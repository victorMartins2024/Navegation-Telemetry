/*-----------------------------------------------------------------

  Telemetry V0.3 main.cpp

  MQtt
  Flash memory
  Display OLED
  LCD 20x4 display I2C 
  RFID: MCRF522
  Keypad 4x4 I2C 
  
  Compiler: VsCode 1.88.1
  MCU: ESP32 DEV MODULE

  Author: Victor Martins
  date: 2024, May

-----------------------------------------------------------------*/

// ----------------------------------------------------------------- 
// ---Libraries---
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "LiquidCrystal_I2C.h"
#include "freertos/task.h"
#include "PubSubClient.h"
#include "esp_task_wdt.h"
#include "Preferences.h"
#include "Password.h"
#include "rtc_wdt.h"
#include "Arduino.h"
#include "MFRC522.h"
#include "WiFi.h"
#include "Wire.h"
#include "SPI.h"
#include "Keypad.h"
#include "Keypad_I2C.h"
// -----------------------------------------------------------

// --------------------------------------s---------------------
#define LCDADRESS 0x27
#define I2CADDR 0x21
// ----------------------------------------------------------------

// ---------------------------------------------------------------- 
//----pins----
#define SS_PIN 1
#define RST_PIN 3
// -----------------------------------------------------------------  

// ----------------------------------------------------------------- 
// ---connection infos--
const char *ssid    =    "---------------------";                   // wifi name      
const char *pass    =    "---------------";                         // wifi password
const char *mqtt    =    "---------------";                         // broker IP
const char *user    =    "-----------";                             // broker user      
const char *passwd  =    "-----------";                             // broker password     
int port  =  1883;
// -----------------------------------------------------------------

// -----------------------------------------------------------------  
// -----Topics----- 
const char *topic_user    =    "sinal/sim/rfid";
const char *topic_CAD     =    "sinal/sim/Cadastro";
const char *topic_TEC     =    "sinal/sim/manutenção";  
const char *topic_V       =    "sinal/sim/checklist/P1:";        // broker topic Vazamento           
const char *topic_G       =    "sinal/sim/checklist/P2:";        // broker topic Garfo              
const char *topic_E       =    "sinal/sim/checklist/P3:";        // broker topic Emergencia       
const char *topic_F       =    "sinal/sim/checklist/P4:";        // broker topic Frente e Ré     
const char *topic_B       =    "sinal/sim/checklist/P5:";        // broker topic Bateria, cabos, conectores
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// --Preferences Key---
const char *prevpref = "Manupreve";
const char *correpref = "Manucorre";
const char *cadaspref = "Cadastro";
const char *listapref = "Cadastro Cartoes";
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// ----Variables----
char uid_buffer[32];
char CAD[32];
bool acertou;
bool navcheck;
bool value = true;
int preve;
int corre;
int marcadorNovoCartao = 0;
byte a = 7;
byte b = 5;
byte maxPasswordLength = 5;
byte currentPasswordLength = 0;
byte PassLenghtMax = 6;
byte PassLenghtAtual = 0;
String cartoesCadastrados = "";  // Salvar os cartões cadastrados
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// ----Definition keypad----  
const byte ROWS = 4;
const byte COLS = 4;
// -----------------------------------------------------------------

// -----------------------------------------------------------------  
//----pins Keypad------
byte rowPins[ROWS] = { 0, 1, 2, 3 };
byte colPins[COLS] = { 4, 5, 6, 7 };
// ----------------------------------------------------------------- 

// -----------------------------------------------------------------
// ----Keypad---- 
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// ----------------------------------------------------------------- 

// -----------------------------------------------------------------
// UIDS
String OpeCard  =  "6379CF9A";  
String AdmCard  =  "29471BC2";  
String TecCard  =  "D2229A1B";  
String PesCard  =  "B2B4BF1B";
String card     =  "E6A1191E";
String battery2 =  "F4E0A08D";
// -----------------------------------------------------------------
char pss[]  = "2552";
// -----------------------------------------------------------------
// -----Objects----
Preferences prefs; 
WiFiClient ESP32Clientv; 
MFRC522 rfid(SS, RST_PIN); 
PubSubClient client(ESP32Clientv); 
Password password = Password(pss);  
LiquidCrystal_I2C lcd(0x27, 20, 4);
Keypad_I2C Keypad_I2C(I2CADDR, 21);
Keypad kpd(makeKeymap(keys), rowPins, colPins, ROWS, COLS);  
// ----------------------------------------------------------------- 

// -----------------------------------------------------------------
// ----Functions---- 
void recon();
void eng();
void processNumberKey(char key);
void aprovadoPass();
void resetPassword();
void telas();
void cadastrar();
void CadastrarCartao();
void formatar();
void excluir();
void dell();
void tag(char key);
void apx();
void vazamento();
void garfos();
void emergencia();
void comando();
void bateria();
void manutencao();
void status();
void screens();
void telafinal();
// ----------------------------------------------------------------- 

// -----------------------------------------------------------------
// ---- ---- 
extern "C" void app_main(){
  Wire.begin();
  Serial.begin(115200);
  Keypad_I2C.begin();
  kpd.begin(makeKeymap(keys));
  WiFi.begin(ssid, pass);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  prefs.begin("manu",false);

  lcd.clear();
  lcd.setCursor(3, 2);
  lcd.print("INICIALIZANDO");
  vTaskDelay(900);

  preve = prefs.getInt(prevpref, preve);
  corre = prefs.getInt(correpref, corre);
   
  // connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
    vTaskDelay(1000);
  }

  // connect to broker
  client.setServer(mqtt, port);
  while (!client.connected()) {
    client.connect("ESP32Clientv", user, passwd);
    vTaskDelay(500);
  }
  
  while(1){

    client.loop();

  char menu = kpd.getKey();

  if (preve == 1) {
    status();
  } else if (corre == 1) {
    status();
  } else {
    if (menu != NO_KEY) {
      vTaskDelay(80);
      if (menu == '0') {
        navcheck = false;  // Para tela de Engenharia
        eng();
      }
    } else {
      navcheck = true;  // Para tela de Operação
      apx();
    }
  }
    vTaskDelay(1000);
  }    
}
//======================================================================TELAS ENGENHARIA======================================================================//
void eng() {

  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("PASSWORD:");
  lcd.setCursor(14, 3);
  lcd.print("#-SAIR");

  while (1) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == 'C') {
        resetPassword();
      } else if (key == '#') {
        apx();
      } else if (key == 'D') {
        if (value == true) {
          aprovadoPass();
          value = false;
        }
      } else {
        processNumberKey(key);
      }
    }
  }
}

void processNumberKey(char key) {

  lcd.setCursor(a, 2);
  lcd.print("*");
  a++;
  if (a == 11) {
    a = 4;  // Tamanho da senha com 4 digitos "2552"
  }
  currentPasswordLength++;
  password.append(key);

  if (currentPasswordLength == maxPasswordLength) {
    aprovadoPass();
  }
}

void aprovadoPass() {

  currentPasswordLength = 0;

  if (password.evaluate()) {
    lcd.clear();
    lcd.setCursor(7, 2);
    lcd.print("VALIDO");
    vTaskDelay(1000);
    a = 7;
    acertou = true;  // mostra que acertou, apaga a mensagem anterior e segue para a tela screens ou  telas
    if (navcheck == true) {
      screens();
    } else {
      telas();
    }
  } else {
    lcd.clear();
    lcd.setCursor(6, 1);
    lcd.print("INVALIDO");
    lcd.setCursor(2, 2);
    lcd.print("TENTE NOVAMENTE");
    vTaskDelay(1000);
    a = 7;
    acertou = false;  // mostra que acertou, apaga a mensagem anterior e  volta para a tela de Password e colocar a senha correta
  }
  resetPassword();
}

void resetPassword() {

  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  a = 7;

  if (acertou != true) {
    eng();  // Mostra que errou a senha, apaga a senha sem sumir a mensagem "Password"
  }
}

void telas() {

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("ESCOLHA A OPCAO:");
  lcd.setCursor(0, 1);
  lcd.print("1- FORMATAR");
  lcd.setCursor(0, 2);
  lcd.print("2- EXCLUIR");
  lcd.setCursor(0, 3);
  lcd.print("#- SAIR");

  while (1) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        formatar();
      } else if (key == '2') {
        excluir();
      } else if (key == '#') {
        navcheck = true;  // True, após apertar "#" ele habilita a opção para bater os RFIDs novamente para seguir com outras opções.
        apx();
      }
    }
  }
}

void cadastrar() {

  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("TAG:");
  lcd.setCursor(1, 2);
  lcd.print("RFID:");
  lcd.setCursor(14, 3);
  lcd.print("#-SAIR");
  acertou = false;

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == 'C') {
        dell();
      } else if (key == '#') {
        b = 5;
        screens();
      } else {
        tag(key);
      }
    }
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      snprintf(CAD, sizeof(CAD), "%02X%02X%02X%02X",
               rfid.uid.uidByte[0], rfid.uid.uidByte[1],
               rfid.uid.uidByte[2], rfid.uid.uidByte[3]);

      b = 5;  // Sempre que bater o RFID a proxima TAG volta ser digitada na posição 5
      client.publish(topic_CAD, "");
      lcd.setCursor(6, 2);
      lcd.print(CAD);
      vTaskDelay(1000);
      cadastrar();
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void CadastrarCartao() {

  String conteudo = "";

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    snprintf(CAD, sizeof(CAD), "%02X%02X%02X%02X",
             rfid.uid.uidByte[0], rfid.uid.uidByte[1],
             rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
    conteudo.concat(CAD);  // Juntando os valores do CAD no conteudo
    conteudo = conteudo + ";";
    cartoesCadastrados = cartoesCadastrados + conteudo;


    prefs.putString(listapref, cartoesCadastrados);
  }
}

void formatar() {

  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("FORMARTAR?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (1) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        lcd.clear();
        lcd.setCursor(5, 2);
        lcd.print("FORMATADO");
        vTaskDelay(1000);
        telas();
      } else if (key == '2') {
        telas();
      }
    }  // João
  }
}

void excluir() {

  lcd.clear();
  lcd.setCursor(1, 2);
  lcd.print("APROXIME O CARTAO");

  while (1) {

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      snprintf(uid_buffer, sizeof(uid_buffer), "%02X%02X%02X%02X",
               rfid.uid.uidByte[0], rfid.uid.uidByte[1],
               rfid.uid.uidByte[2], rfid.uid.uidByte[3]);

      client.publish(topic_CAD, uid_buffer);

      if (strcmp(uid_buffer, PesCard.c_str()) == 0) {
        lcd.clear();
        lcd.setCursor(7, 2);
        lcd.print("APAGADO");
        vTaskDelay(1000);
        PesCard = "";
        if (navcheck == true) {
          screens();
        } else {
          telas();
        }
      } else {
        lcd.clear();
        lcd.setCursor(5, 1);
        lcd.print("CARTAO NAO");
        lcd.setCursor(5, 2);
        lcd.print("CADASTRADO");
        vTaskDelay(1000);
        if (navcheck == true) {
          screens();
        } else {
          telas();
        }
      }
    }
  }
}

void dell() {

  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  b = 5;

  if (acertou != true) {
    cadastrar();
  }
}

void tag(char key) {

  lcd.setCursor(b, 1);
  lcd.print(key);
  b++;

  if (b == 11) {
    b = 5;  // Tamanho da TAG com 6 digitos "333..."
  }
  PassLenghtAtual++;
  password.append(key);
}
//=======================================================================TELAS OPERAÇÃO========================================================================//
void apx() {

  lcd.clear();
  lcd.setCursor(2, 2);
  lcd.print("APROXIMAR CARTAO");

  while (navcheck == true) {

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      snprintf(uid_buffer, sizeof(uid_buffer), "%02X%02X%02X%02X",
               rfid.uid.uidByte[0], rfid.uid.uidByte[1],
               rfid.uid.uidByte[2], rfid.uid.uidByte[3]);

      client.publish(topic_user, uid_buffer);

      if (strcmp(uid_buffer, OpeCard.c_str()) == 0) {
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("OPERADOR");
        lcd.setCursor(4, 2);
        lcd.print("IDENTIFICADO");
        vTaskDelay(1000);
        vazamento();
      } else if (strcmp(uid_buffer, TecCard.c_str()) == 0) {
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("TECNICO");
        lcd.setCursor(4, 2);
        lcd.print("IDENTIFICADO");
        vTaskDelay(1000);
        manutencao();
      } else if (strcmp(uid_buffer, AdmCard.c_str()) == 0) {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("ADMINISTRADOR");
        lcd.setCursor(4, 2);
        lcd.print("IDENTIFICADO");
        vTaskDelay(1000);
        eng();
      }
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void vazamento() {

  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("VAZAMENTO?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        client.publish(topic_V, "True");
        garfos();
      } else if (key == '2') {
        client.publish(topic_V, "False");
        garfos();
      }
    }
    if (WiFi.status() != WL_CONNECTED  || !client.connected())
        recon();
  }
}

void garfos() {

  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("GARFOS?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        client.publish(topic_G, "True");
        emergencia();
      } else if (key == '2') {
        client.publish(topic_G, "False");
        emergencia();
      }
    }
  }
}

void emergencia() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOTAO DE EMERGENCIA?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        client.publish(topic_E, "True");
        comando();
      } else if (key == '2') {
        client.publish(topic_E, "False");
        comando();
      }
    }
  }
}

void comando() {

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("FRENTE E RE?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        client.publish(topic_F, "True");
        bateria();
      } else if (key == '2') {
        client.publish(topic_F, "False");
        bateria();
      }
    }
  }
}

void bateria() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BATERIA,CABOS,CONEC?");
  lcd.setCursor(0, 2);
  lcd.print("1 - SIM");
  lcd.setCursor(0, 3);
  lcd.print("2 - NAO");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        client.publish(topic_B, "True");
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("CONCLUIDO");
        lcd.setCursor(2, 2);
        lcd.print("MAQUINA LIBERADA");
        vTaskDelay(1000);
        navcheck = true;  // "Mudar futuramente de 'true' para 'false' quando for para tela de funcionando", deixando true ele não zera todos para false após repetir o checklist novamente
        telafinal();
      } else if (key == '2') {
        client.publish(topic_B, "False");
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("CONCLUIDO");
        lcd.setCursor(2, 2);
        lcd.print("MAQUINA LIBERADA");
        vTaskDelay(1000);
        navcheck = true;  // "Mudar futuramente de 'true' para 'false' quando for para tela de funcionando", deixando true ele não zera todos para false após repetir o checklist novamente
        telafinal();
      }
    }
    corre = 0;
    preve = 0;
    vTaskDelay(50);
    prefs.putInt(correpref, corre);
    prefs.putInt(prevpref, preve);
  }
}

void manutencao() {

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("ESCOLHA A OPCAO:");
  lcd.setCursor(0, 1);
  lcd.print("1- MANU PREVENTIVA");
  lcd.setCursor(0, 2);
  lcd.print("2- MANU CORRETIVA");
  lcd.setCursor(14, 3);
  lcd.print("#-SAIR");
  vTaskDelay(80);

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        preve = 1;
        vTaskDelay(30);
        prefs.putInt(prevpref, preve);
        status();
      } else if (key == '2') {
        corre = 1;
        vTaskDelay(30);
        prefs.putInt(correpref, corre);
        status();
      } else if (key == '#') {
        corre = 0;
        preve = 0;
        vTaskDelay(30);
        prefs.putInt(correpref, corre);
        prefs.putInt(prevpref, preve);
        apx();
      }
    }
  }
}

void status() {

  lcd.clear();

  while (1) {

    lcd.setCursor(6, 0);
    lcd.print("MAQUINA");
    lcd.setCursor(9, 1);
    lcd.print("EM");
    lcd.setCursor(5, 2);
    lcd.print("MANUTENCAO");

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      char SAIR[32];
      snprintf(SAIR, sizeof(SAIR), "%02X%02X%02X%02X",
               rfid.uid.uidByte[0], rfid.uid.uidByte[1],
               rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
      client.publish(topic_user, SAIR);

      if (strcmp(SAIR, TecCard.c_str()) == 0) {
        corre = 0;
        preve = 0;
        prefs.putInt(correpref, corre);
        prefs.putInt(prevpref, preve);
        client.publish(topic_TEC, " ");
        navcheck = true;
        vTaskDelay(80);
        apx();
      }
    } else if (!rfid.PICC_IsNewCardPresent() && !rfid.PICC_ReadCardSerial()) {

      if (preve == 1) {
        client.publish(topic_TEC, "Preventiva");
        lcd.setCursor(5, 3);
        lcd.print("PREVENTIVA");
        vTaskDelay(40);
      } else if (corre == 1) {
        client.publish(topic_TEC, "Corretiva");
        lcd.setCursor(5, 3);
        lcd.print("CORRETIVA");
        vTaskDelay(40);
      }
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void screens() {

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("ESCOLHA A OPCAO:");
  lcd.setCursor(0, 1);
  lcd.print("1- CADASTRAR");
  lcd.setCursor(0, 2);
  lcd.print("2- EXCLUIR");
  lcd.setCursor(0, 3);
  lcd.print("#- SAIR");

  while (navcheck == true) {

    char key = kpd.getKey();

    if (key != NO_KEY) {
      vTaskDelay(20);
      if (key == '1') {
        navcheck = true;
        cadastrar();
      } else if (key == '2') {
        navcheck = true;
        excluir();
      } else if (key == '#') {
        apx();
      }
    }
  }
}

void telafinal() {

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("NMU-V0.3");
  lcd.setCursor(5, 1);
  lcd.print("EMPRESA IHM");
  lcd.setCursor(3, 2);
  lcd.print("OPERANDO");
  lcd.setCursor(0, 3);
  lcd.print("SHOWROOM-SP");
  vTaskDelay(1500);
 
}
/*---------------------------------------
----------------FUNCTIONS----------------
---------------------------------------*/
void recon(){
  while (WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();        
    vTaskDelay(1000);        
    WiFi.begin(ssid, pass);  
    vTaskDelay(1000);        
  }        
  vTaskDelay(500);
  while (!client.connected()){                        
    client.connect("ESP32Client", user, passwd);    
    vTaskDelay(5000);                                  
  }                    
}