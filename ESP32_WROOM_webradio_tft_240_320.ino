/*
* MIT License
*
* Copyright (c) 2023 thieu-b55
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
/*
 * ESP32 WROOM Module
 * Partition Scheme:
 * Huge APP(3MB No OTA/1MB SPIFSS)
 *  
 *  
 * PCM5102A 
 * FLT  >>  GND
 * DMP  >>  3.3V
 * SCL  >>  GND
 * BCK  >>  GPIO27 ESP32
 * DIN  >>  GPIO25 ESP32
 * LCK  >>  GPIO26 ESP32
 * FMT  >>  GND
 * XMT  >>  3.3V
 * VCC  >>  5V
 * GND  >>  GND
 * 
 * 
 * TFT IL9341 Driver
 * instellingen TFT scherm
 * aanpassen in .../libraries/TFT_eSPI/User_Setup.h
 * TFT_MISO   >>  GPIO12       
 * TFT_MOSI   >>  GPIO13    
 * TFT_CLK    >>  GPIO14    
 * TFT_CS     >>  GPIO15 
 * TOUCH_MISO >>  GPIO12
 * TOUCH_MOSI >>  GPIO13
 * TOUCH_CLK  >>  GPIO14
 * TOUCH_CS   >>  GPIO04
 * TFT_DC     >>  GPIO02
 * TFT_RESET  >>  -1
 *  
 * 
 * SD kaart
 * CS     >> GPIO05 ESP32
 * MOSI   >> GPIO23 ESP32
 * MISO   >> GPIO19 ESP32
 * SCK    >> GPIO18 ESP32
 * 
 * Audio librarie
 * https://github.com/schreibfaul1/ESP32-audioI2S
 * 
 * 
 * 
 * zender_data.csv
 * geen header
 * kolom 1  >>  zendernaam
 * kolom2   >>  zender url
*/

#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Preferences.h>
#include "FS.h"
#include "SD.h"
#include <CSV_Parser.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

TFT_eSPI tft = TFT_eSPI();
Audio audio;
Preferences pref;
AsyncWebServer server(80);

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

//PCM1502A
#define I2S_DOUT            25
#define I2S_BCLK            27
#define I2S_LRC             26

//SD kaart
#define SD_SCK              18
#define SD_MISO             19
#define SD_MOSI             23
#define SD_CS               5

//TFT scherm en touch
#define TFT_MISO            12
#define TFT_MOSI            13
#define TFT_SCK             14
#define TFT_CS              15
#define TOUCH_CS            4
#define TOUCH_INT           32

//Rotary switch station
#define STATION_A           34
#define STATION_B           35
#define STATION_OK          33 

//Rotary switch volume
#define VOLUME_A            16
#define VOLUME_B            17

#define MAX_AANTAL_KANALEN  75

int gekozen = 1;
int keuze = 1;
int keuze_vorig = 1;
int gn_keuze_vorig = 1;
int volgend;
int totaalmp3;
int eerste;
int tweede;
int songindex;
int row;
int volume_keuze;
int volume_gekozen;
int laag_keuze;
int laag_gekozen;
int midden_keuze;
int midden_gekozen;
int hoog_gekozen;
int hoog_keuze;
int mp3_per_songlijst;
int array_index = MAX_AANTAL_KANALEN - 1;
int songlijst_index_vorig;
int songlijst_index;
int mp3_folder_teller;
int teller = 0;
int mp3_aantal;
int gn_keuze = 0;
int ip_1_int = 192;
int ip_2_int = 168;
int ip_3_int = 1;
int ip_4_int = 177;
int tft_positie_int = 10;
int tft_positie_plus_int = 10;
uint16_t t_x;
uint16_t t_y; 
unsigned long wacht_op_netwerk;
unsigned long inlezen_begin;
unsigned long inlezen_nu;
unsigned long wachttijd;;
unsigned long tft_update_long;
unsigned long touch_update_long;
unsigned long tft_bericht_long;
unsigned long int_wachttijd_long = 111;
unsigned long int_begin_long;
bool eerste_bool = false;
bool kiezen = false;
bool web_kiezen = false;
bool lijst_maken = false;
bool speel_mp3 = false;
bool webradio = false;
bool schrijf_csv = false;
bool netwerk;
bool nog_mp3;
bool mp3_ok;
bool mp3_lijst_maken = false;
bool ssid_ingevuld = false;
bool pswd_ingevuld = false;
bool songlijsten = false;
bool songlijst_bestaat_bool;
bool touch_int_bool = false;
bool int_bool = false;
bool gekozen_bool = false;
bool pressed;
bool station_A_bool = false;
bool station_B_bool = false;
bool station_plus_bool = false;
bool station_min_bool = false;
bool volume_A_bool = false;
bool volume_B_bool = false;
bool volume_plus_bool = false;
bool volume_min_bool = false;
bool volume_bewaren_bool = false;
char ip_char[20];
char songfile[200];
char mp3file[200];
char song[200];
char datastring[200];
char password[40];
char ssid[40];
char charZenderFile[12];
char speler[20];
char gn_actie[20];
char gn_selectie[20];
char zendernaam[40];
char charUrlFile[12];
char url[100];
char mp3_dir[10];
char folder_mp3[10];
char aantal_mp3[10];
char songlijst_dir[12];
char totaal_mp3[15];
char mp3_lijst_folder[10];
char mp3_lijst_aantal[5];
char leeg[0];
char zenderarray[MAX_AANTAL_KANALEN][40];
char urlarray[MAX_AANTAL_KANALEN][100];
char hoeveel_mp3[5];
const char* IP_1_KEUZE = "ip_1_keuze";
const char* IP_2_KEUZE = "ip_2_keuze";
const char* IP_3_KEUZE = "ip_3_keuze";
const char* IP_4_KEUZE = "ip_4_keuze";
const char* KEUZEMIN_INPUT = "minKeuze";
const char* KEUZEPLUS_INPUT = "plusKeuze";
const char* BEVESTIGKEUZE_INPUT ="bevestigKeuze";
const char* LAAG = "laag_keuze";
const char* MIDDEN = "midden_keuze";
const char* HOOG = "hoog_keuze";
const char* VOLUME = "volume_keuze";
const char* VOLUME_BEVESTIG = "bevestig_volume";
const char* APssid = "ESP32webradio";
const char* APpswd = "ESP32pswd";
const char* STA_SSID = "ssid";
const char* STA_PSWD = "pswd";
const char* ZENDER = "zender";
const char* URL = "url";
const char* ARRAY_MIN = "array_index_min";
const char* ARRAY_PLUS = "array_index_plus";
const char* BEVESTIG_ZENDER = "bevestig_zender";
const char* MIN_INPUT = "min";
const char* PLUS_INPUT = "plus";
const char* BEVESTIG_MP3 ="bevestig_mp3";
const char* h_char = "h";
String songstring =      "                                                                                                                                                                                                        ";
String inputString =     "                                                                                                                                                                                                        ";
String mp3titel =        "                                                                                                                                                                                                        ";      
String zenderFile =      "           ";
String urlFile =         "           ";
String maxurl =          "           ";
String totaal =          "           ";
String streamsong =      "                                                                                                                                                                                                        ";
String mp3_folder =      "                   ";
String songlijst_folder = "                   ";
String mp3test = "mp3";
String ip_1_string = "   ";
String ip_2_string = "   ";
String ip_3_string = "   ";
String ip_4_string = "   ";
String ip_string = "                   ";
String mp3_per_folder = "      ";

void readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file){
      Serial.println("Kan file niet openen om te lezen : ");
      Serial.println(path);
      return;
    }
    teller = 0;
    inputString = "";
    while(file.available()){
      inputString += char(file.read());
      teller++;
    }
    file.close();
}

void readIP(fs::FS &fs, const char * path){
    int temp;
    int temp1;
    File file = fs.open(path);
    if(!file){
      return;
    }
    teller = 0;
    inputString = "";
    while(file.available()){
      inputString += char(file.read());
      teller++;
    }
    temp = inputString.indexOf('.');
    ip_1_int = (inputString.substring(0, temp - 1)).toInt();
    temp1 = inputString.indexOf('.', temp + 1);
    ip_2_int = (inputString.substring(temp + 1 , temp1 - 1)).toInt();
    temp = inputString.indexOf('.', temp1 + 1);
    ip_3_int = (inputString.substring(temp1 + 1, temp - 1)).toInt();
    ip_4_int = (inputString.substring(temp + 1, inputString.length() - 1)).toInt();
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
  fs.remove(path);
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  file.print(message);
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Kan file niet openen om te schrijven : ");
      Serial.println(path);
      return;
  }
  file.print(message);
  file.close();
}

void testDir(fs::FS &fs, const char * path){
  File root = fs.open(path);
  if(root){
    songlijst_bestaat_bool = true;
  }
}

void createDir(fs::FS &fs, const char * path){
  File root = fs.open(path);
  if(root){
    songlijsten = true;
    lijst_maken = false;
    while(1){
      yield();
    }
  }
  else{
    fs.mkdir(path);
  }
}

void audio_showstreamtitle(const char *info){
  if(kiezen == false){
    streamsong = info;
  }
}

void audio_eof_mp3(const char *info){
  mp3_volgend();
  streamsong = mp3titel.substring(0, (mp3titel.length() - 4));
}

void files_in_mp3_0(fs::FS &fs, const char * dirname, uint8_t levels){
  File root = fs.open(dirname);
  if(!root){
    Serial.println("Geen mp3_0 folder");
    return;
  }
  File file = root.openNextFile();
  mp3_per_songlijst = 0;
  while(file){
    file = root.openNextFile();
    mp3_per_songlijst ++;
  }
  String(mp3_per_songlijst).toCharArray(hoeveel_mp3, (String(mp3_per_songlijst).length() +1));
  writeFile(SD, "/files", hoeveel_mp3);
}

void maak_lijst(fs::FS &fs, const char * dirname){
  File root = fs.open(dirname);
  if(!root){
    nog_mp3 = false;
    return;
  }
  File file = root.openNextFile();
  while(file){
    songlijst_index =  mp3_aantal / mp3_per_songlijst;
    if(songlijst_index != songlijst_index_vorig){
      songlijst_index_vorig = songlijst_index;
      songlijst_folder = "/songlijst" + String(songlijst_index);
      songlijst_folder.toCharArray(songlijst_dir, (songlijst_folder.length() + 1));
      createDir(SD, songlijst_dir);
    }
    songstring = file.name();
    songlijst_folder = "/songlijst" + String(songlijst_index) + "/s" + String(mp3_aantal);
    songlijst_folder.toCharArray(songlijst_dir, (songlijst_folder.length() + 1));
    songstring = file.name();
    songstring.toCharArray(song, (songstring.length() + 1));
    writeFile(SD, songlijst_dir, song);
    file = root.openNextFile();
    mp3_aantal ++;
  }
}

void mp3_lijst_maken_gekozen(){
  inlezen_begin = millis();
  files_in_mp3_0(SD, "/mp3_0", 1);
  mp3_aantal = 0;
  nog_mp3 = true;
  mp3_folder_teller = 0;
  songlijst_index_vorig = -1;
  while(nog_mp3){
    mp3_folder = "/mp3_" + String(mp3_folder_teller);
    inlezen_nu = millis() - inlezen_begin;
    Serial.println(mp3_aantal);
    Serial.println(inlezen_nu);
    mp3_folder.toCharArray(mp3_dir, (mp3_folder.length() + 1));
    maak_lijst(SD, mp3_dir);
    mp3_folder_teller ++;
  }
  String(mp3_aantal).toCharArray(totaal_mp3, (String(mp3_aantal - 1).length() +1));
  writeFile(SD, "/totaal", totaal_mp3);
  int verstreken_tijd = (millis() - inlezen_begin) / 1000;
  Serial.println("verstreken tijd");
  Serial.println(verstreken_tijd);
  lijst_maken = false;
  keuze = -1;
  gekozen = -1;
  mp3_gekozen();
}

void mp3_gekozen(){
  readFile(SD, "/totaal");
  totaal = inputString.substring(0, teller);
  totaalmp3 = totaal.toInt();
  readFile(SD, "/files");
  mp3_per_folder = inputString.substring(0, teller);
  mp3_per_songlijst = mp3_per_folder.toInt();
  mp3_volgend();
}

void mp3_volgend(){
  mp3_ok = false;
  while(mp3_ok == false){
    volgend = random(totaalmp3);
    songindex = volgend / mp3_per_songlijst;
    songstring = "/songlijst" + String(songindex) + "/s" + String(volgend);
    songstring.toCharArray(songfile, (songstring.length() +1));
    readFile(SD, songfile);
    inputString.toCharArray(mp3file, inputString.length() + 1);
    mp3_ok = audio.connecttoFS(SD, mp3file);
  }
  songstring = String(mp3file);
  eerste = songstring.indexOf("/");
  tweede = songstring.indexOf("/", eerste + 1);
  mp3titel = songstring.substring(tweede + 1);
  streamsong = mp3titel.substring(0, (mp3titel.length() - 4));
}

void radio_gekozen(){
  memset(url, 0, sizeof(url));
  strcpy(url, urlarray[gekozen]);
  memset(zendernaam, 0, sizeof(zendernaam));
  strcpy(zendernaam, zenderarray[gekozen]);
  audio.connecttohost(url);
}

void schrijf_naar_csv(){
  char terminator = char(0x0a);
  String datastring = "                                                                                                                                                             ";
  char datastr[150];
  deleteFile(SD, "/zender_data.csv");
  for(int x = 0; x < MAX_AANTAL_KANALEN; x++){
    datastring = String(zenderarray[x]) + "," + String(urlarray[x]) + String(terminator);
    datastring.toCharArray(datastr, (datastring.length() + 1));
    appendFile(SD, "/zender_data.csv", datastr);
  }
  lees_CSV();
}

/*
 * Inlezen CSV file naar zender en url arry
 */
void lees_CSV(){
  CSV_Parser cp("ss", false, ',');
  if(cp.readSDfile("/zender_data.csv")){
    char **station_naam = (char**)cp[0];  
    char **station_url = (char**)cp[1]; 
    for(row = 0; row < cp.getRowsCount(); row++){
      memset(zenderarray[row], 0, sizeof(zenderarray[row]));
      strcpy(zenderarray[row], station_naam[row]);
      memset(urlarray[row], 0, sizeof(urlarray[row]));
      strcpy(urlarray[row], station_url[row]);
    }
  }
}

/*
 * Copied from TFT_eSPI example files
 */
void touch_calibrate(){
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  if (!SPIFFS.begin()){
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }
  if (SPIFFS.exists(CALIBRATION_FILE)){
    if (REPEAT_CAL){
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else{
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f){
        if (f.readBytes((char *)calData, 14) == 14){
          calDataOK = 1;
        }
        f.close();
      }
    }
  }
  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } 
  else{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Touch corners as indicated");
    tft.setTextFont(1);
    tft.println();
    if (REPEAT_CAL){
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f){
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void touch(){
  detachInterrupt(digitalPinToInterrupt(TOUCH_INT));
  touch_int_bool = true;
}

/*
 * Flushen nog aanwezige interrupt
 */
void dummy(){
  
}

void station_a_int(){
  detachInterrupt(digitalPinToInterrupt(STATION_A));
  int_begin_long = millis();
  if(!station_A_bool){
    if(!digitalRead(STATION_B)){
      station_plus_bool = true;
    }
    else{
      station_min_bool = true;
    }
    station_A_bool = true;
  }
}

void station_b_int(){
  detachInterrupt(digitalPinToInterrupt(STATION_B));
  station_B_bool = true;
}

void volume_a_int(){
  detachInterrupt(digitalPinToInterrupt(VOLUME_A));
  int_begin_long = millis();
  if(!volume_A_bool){
    if(!digitalRead(VOLUME_B)){
      volume_plus_bool = true;
    }
    else{
      volume_min_bool = true;
    }
    volume_A_bool = true;
  }
}

void volume_b_int(){
  detachInterrupt(digitalPinToInterrupt(VOLUME_B));
  volume_B_bool = true;
}

void netwerk_station_wis_tft(){
  SPI.end();
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);         // SPI TFT  
  tft.setTextColor(TFT_BLACK);
  tft.setTextFont(2);
  if(keuze == - 1){
    tft.setCursor(121, 80);
    tft.print("mp3 speler");
  }
  else if(keuze == -2){
    tft.setCursor(114, 80);
    tft.print("mp3 lijst maken");
  }
  else{
    tft.setCursor((320 - (String(zenderarray[keuze]).length() * 7)) / 2, 80);
    tft.print(zenderarray[keuze]);
  }
}

void netwerk_station_schrijf_tft(){
  tft.setTextColor(TFT_SKYBLUE);
  tft.setTextFont(2);
  if(keuze == - 1){
    tft.setCursor(121, 80);
    tft.print("mp3 speler");
  }
  else if(keuze == -2){
    tft.setCursor(114, 80);
    tft.print("mp3 lijst maken");
  }
  else{
    tft.setCursor((320 - (String(zenderarray[keuze]).length() * 7)) / 2, 80);
    tft.print(zenderarray[keuze]);
  }
  SPI.end();
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);           // SPI SD kaart
}

void gn_netwerk_station_wis_tft(){
  SPI.end();
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);         // SPI TFT  
  tft.setTextColor(TFT_BLACK);
  tft.setTextFont(2);
  if(gn_keuze == 0){
    tft.setCursor(113, 80);
    tft.print("stop mp3 speler");
  }
  if(gn_keuze == 1){
    tft.setCursor(121, 80);
    tft.print("mp3 speler");
  }
  if(gn_keuze == 2){
    tft.setCursor(114, 80);
    tft.print("mp3 lijst maken");
  }
}

void gn_netwerk_station_schrijf_tft(){
  tft.setTextColor(TFT_SKYBLUE);
  tft.setTextFont(2);
  if(gn_keuze == 0){
    tft.setCursor(113, 80);
    tft.print("stop mp3 speler");
  }
  if(gn_keuze == 1){
    tft.setCursor(121, 80);
    tft.print("mp3 speler");
  }
  if(gn_keuze == 2){
    tft.setCursor(114, 80);
    tft.print("mp3 lijst maken");
  }
  SPI.end();
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);           // SPI SD kaart
}


/*
 * INTERNET RADIO INSTELLINGEN
 */
const char index_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Internetradio bediening</title>
    <meta name="viewport" content="width=device-width, initial-scale=.85">
    <style>
        div.kader {
          position: relative;
          left: 0px;
          width: 400px;
        }
        div.kader_2 {
          position: absolute;
          left : 80px;
          width: 80px;
        }
        div.kader_3 {
          position: absolute;
          left : 160px;
          width: 80px;
        }
        div.kader_4 {
          position: absolute;
          left : 240px;
          width: 80px;
        }
        div.kader_4A {
          position: absolute;
          left : 240px;
          width: 100px;
        }
        div.blanco_10{
          width: auto;
          height: 10px;
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_30{
          width: auto;
          height: 30px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
        div.blanco_50{
          width: auto;
          height: 50px;
        }
        div.blanco_60{
          width: auto;
          height: 60px;
        }
        div.blanco_80{
          width: auto;
          height: 80px;
        }
    </style>
  </head>
  <body>
    <h3><center> ESP32 internetradio </center></h3>
    <div class="blanco_10">&nbsp;</div>
    <h5><center> %zenderNu% </center></h5>
    <small><p><center>%song%</center></p></small>
    <center>
      <input type="text" style="text-align:center;" value="%selecteren%" name="keuze" size=30>
    </center>
    <div class="blanco_30">&nbsp;</div>
    <form action="/get" target="hidden-form">
      <div class="kader">
        <div class="kader_2">
          <center><input type="submit" name="minKeuze" value="   -   " onclick="bevestig()"></center>
        </div>
        <div class="kader_3">
          <center><input type="submit" name="bevestigKeuze" value="OK" onclick="bevestig()"></center>
        </div>
        <div class="kader_4">
          <center><input type="submit" name="plusKeuze" value="   +   " onclick="bevestig()"></center>
        </div>
      </div>
    </form>
    <div class="blanco_40">&nbsp;</div>
    <p><small><b><center>%tekst1%</center></b></small></p>
    <p><small><center>%tekst2%</center></small></p>
    <p><small><b><center>%tekst3%</center></b></small></p>
    <p><small><center>%tekst4%</center></small></p>
    <p><small><b><center>%tekst5%</center></b></small></p>
    <p><small><center>%tekst6%</center></small></p>
    <div class="kader">
      <div class="kader_2">
        <p><small><b>EQ -40 <-> 6 </b></small></p>
      </div>
      <div class="kader_4A">
        <p><small><b>Volume 0 <->21</b></small></p>
      </div>
    </div>
    <div class="blanco_50">&nbsp;</div>
    <form action="/get" target="hidden-form">
    <small>
      <center>
        <labelfor="dummy">L :</label>
        <input type="text" style="text-align:center;" value="%laag_kiezen%"   name="laag_keuze"   size=1>
        &nbsp;&nbsp;
        <labelfor="dummy">M :</label>
        <input type="text" style="text-align:center;" value="%midden_kiezen%" name="midden_keuze" size=1>
        &nbsp;&nbsp;
        <labelfor="dummy">H :</label>
        <input type="text" style="text-align:center;" value="%hoog_kiezen%"   name="hoog_keuze"   size=1>
        &nbsp;&nbsp;
        <labelfor="dummy">V :</label>
        <input type="text" style="text-align:center;" value="%volume_kiezen%" name="volume_keuze" size=1>
      </center>
    </small>
    <div class="blanco_30">&nbsp;</div>
    <center>
      <input type="submit" name="bevestig_volume" value="OK" onclick="bevestig()">
    </center>
    </form>
    <div class="blanco_20">&nbsp;</div>
    <h5><center> Instellen zender en url : %array_index% </center></h5>
    <form action="/get" target="hidden-form">
    <center>
      <input type= "text" style="text-align:center;" value="%zender%" name="zender" size = 30>
    </center>
    <div class="blanco_10">&nbsp;</div>
    <center>
      <input type= "text" style="text-align:center;" value="%url%" name="url" size = 40>
    </center>
    <div class="blanco_30">&nbsp;</div>
    <div class="kader">
      <div class="kader_2">
        <center><input type="submit" name="array_index_min" value="   -   " onclick="bevestig()"></center>
      </div>
      <div class="kader_3">
        <center><input type="submit" name="bevestig_zender" value="OK" onclick="bevestig()"></center>
      </div>
      <div class="kader_4">
       <input type="submit" name="array_index_plus" value="   +   " onclick="bevestig()"></center>
      </div>
    </div>
    </form>
    <div class="blanco_80">&nbsp;</div>
    <div class="blanco_30">&nbsp;</div>
    <h6>thieu-b55 maart 2023</h6>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";


/*
 * NETWERK INSTELLINGEN
 */
const char netwerk_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Internetradio bediening</title>
    <meta name="viewport" content="width=device-width, initial-scale=.85">
    <style>
        div.kader {
          position: relative;
          width: 400px;
          height: 12x;
        }
        div.links{
          position: absolute;
          left : 0px;
          width; auto;
          height: 12px;
        }
        div.links_midden{
          position:absolute;
          left:  80px;
          width: auto;
          height: 12px; 
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
    </style>
  </head>
  <body>
    <p><small><center>%song%</center></small></p>
    <center>
      <input type="text" style="text-align:center;" value="%selectie%" name="keuze" size=30>
    </center>
      <br>
    <form action="/get" target="hidden-form">
    <center>
      <input type="submit" name="min" value="   -   " onclick="ok()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="plus" value="   +   " onclick="ok()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="bevestig_mp3" value="OK" onclick="ok()">
    </center>
    </form>
    <br>
    <p><small><b><center>%tekst1%</center></b></small></p>
    <p><small><center>%tekst2%</center></small></p>
    <p><small><b><center>%tekst3%</center></b></small></p>
    <p><small><center>%tekst4%</center></small></p>
    <p><small><b><center>%tekst5%</center></b></small></p>
    <p><small><center>%tekst6%</center></small></p>
    <p><small><center><b>EQ -40 <-> 6 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Volume 0 <->21</b></center></small></p>
    <form action="/get" target="hidden-form">
    <small>
    <center>
      <labelfor="dummy">L :</label>
      <input type="text" style="text-align:center;" value="%laag_kiezen%"   name="laag_keuze"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">M :</label>
      <input type="text" style="text-align:center;" value="%midden_kiezen%" name="midden_keuze" size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">H :</label>
      <input type="text" style="text-align:center;" value="%hoog_kiezen%"   name="hoog_keuze"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">V :</label>
      <input type="text" style="text-align:center;" value="%volume_kiezen%" name="volume_keuze" size=1>
    </center>
    </small>
    <br>
    <center>
      <input type="submit" name="bevestig_volume" value="OK" onclick="ok()">
    </center>
    </form>
    <br><br>
    <h5><center><strong>ESP32 Netwerk instellingen</strong></center></h5>
    <form action="/get">
    <small>
    <div class="kader">
      <div class="links"><b>ssid :</b></div>
      <div class="links_midden"><input type="text" style="text-align:center;" name="ssid"></div>
    </div>
    <div class="blanco_40">&nbsp;</div>
    <div class="kader">
      <div class="links"><b>pswd :</b></div>
      <div class="links_midden"><input type="text" style="text-align:center;" name="pswd"></div>
    </div>
    <div class="blanco_20">&nbsp;</div>
    </small>
    <h5><center>Gewenst IP address (default 192.168.1.178)</center></h5>
    <div class="kader">
      <center>
        <input type="text" style="text-align:center;" value="%ip_address_1%" name="ip_1_keuze" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_2%" name="ip_2_keuze" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_3%" name="ip_3_keuze" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_4%" name="ip_4_keuze" size=1>
      </center>
    </div>
    <div class="blanco_20">&nbsp;</div>
    </small>
    <center><input type="submit" value="Bevestig" onclick="ok()"></center>
    </form>
    <script>
      function ok(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";


String processor(const String& var){
  char char_tekst1[30];
  char char_tekst2[30];
  char char_tekst3[30];
  char char_tekst4[30];
  char char_tekst5[30];
  char char_tekst6[30];
  char char_tekst7[40];
  if(var == "zenderNu"){
    if(gekozen == -2){
      String mp3_lijst = "mp3 lijst maken";
      mp3_lijst.toCharArray(speler, (mp3_lijst.length() + 1));
      return(speler);
    }
    else if(gekozen == -1){
      String mp3_speler = "mp3 speler";
      mp3_speler.toCharArray(speler, (mp3_speler.length() + 1));
      return(speler);
    }
    else{
      return(zenderarray[gekozen]);
    }
  }
  if(var == "song"){
    return(streamsong);
  }
  if(var == "selectie"){
    if(gn_keuze == 0){
      String selectie = "Stop mp3 speler";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
    if(gn_keuze == 1){
      String selectie = "mp3 speler";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
    if(gn_keuze == 2){
      String selectie = "Maak mp3 lijst";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
  }
  if(var == "selecteren"){
    if(keuze == - 2){
      String mp3_lijst = "mp3 lijst maken";
      mp3_lijst.toCharArray(speler, (mp3_lijst.length() + 1));
      return(speler);
    }
    else if((keuze == -1) || (gn_keuze == 1)){
      String mp3_speler = "mp3 speler";
      mp3_speler.toCharArray(speler, (mp3_speler.length() + 1));
      return(speler);
    }
    else{
      return(zenderarray[keuze]);
    }
  }
  if(var == "tekst1"){
    if((!lijst_maken) && (!songlijsten)){
      return(leeg);
    }
    if(lijst_maken){
      String tekst1 = "inlezen van : ";
      tekst1.toCharArray(char_tekst1, (tekst1.length() + 1));
      return(char_tekst1);
    }
    if(songlijsten){
      String tekst7 = "EERST ALLE SONGLIJSTXX VERWIJDEREN";
      tekst7.toCharArray(char_tekst7, (tekst7.length() + 1));
      return(char_tekst7);
    }
  }
  if(var == "tekst2"){
    if(!lijst_maken){
      return(leeg);
    }
    else{
      mp3_folder.toCharArray(char_tekst2, (mp3_folder.length() + 1));
      return(char_tekst2);;
    }
  }
 if(var == "tekst3"){
    if(!lijst_maken){
      return(leeg);
    }
    else{
      String tekst3 = "aantal mp3's ingelezen : ";
      tekst3.toCharArray(char_tekst3, (tekst3.length() + 1));
      return(char_tekst3);
    }
  }
  if(var == "tekst4"){
    if(!lijst_maken){
      return(leeg);
    }
    else{
      String tekst4 = String(mp3_aantal);
      tekst4.toCharArray(char_tekst4, (tekst4.length() + 1));
      return(char_tekst4);
    }
  }
  if(var == "tekst5"){
    if(!lijst_maken){
      return(leeg);
    }
    else{
      String tekst5 = "seconden reeds bezig : ";
      tekst5.toCharArray(char_tekst5, (tekst5.length() + 1));
      return(char_tekst5);
    }
  }
  
  if(var == "tekst6"){
    if(!lijst_maken){
      return(leeg);
    }
    else{
      int seconden = (millis() - inlezen_begin) / 1000;
      String(seconden).toCharArray(char_tekst6, (String(seconden).length() + 1));
      return(char_tekst6);
    }
  }
  if(var == "laag_kiezen"){
    return(String(laag_gekozen));
  }
  if(var == "midden_kiezen"){
    return(String(midden_gekozen));
  }
  if(var == "hoog_kiezen"){
    return(String(hoog_gekozen));
  }
  if(var == "volume_kiezen"){
    return(String(volume_gekozen));
  }
  if(var == "array_index"){
    return(String(array_index));
  }
  if(var == "zender"){
    return(zenderarray[array_index]);
  }
  if(var == "url"){
    return(urlarray[array_index]);
  }
  if(var == "folder"){
    String folder = mp3_folder;
    folder.toCharArray(mp3_lijst_folder, (folder.length() + 1));
    return(mp3_lijst_folder);
  }
  if(var == "mp3"){
    String aantal = String(mp3_aantal);
    aantal.toCharArray(mp3_lijst_aantal, (aantal.length() + 1));
    return(mp3_lijst_folder);
  }
  if(var == "ip_address_1"){
    return(String(ip_1_int));
  }
  if(var == "ip_address_2"){
    return(String(ip_2_int));
  }
  if(var == "ip_address_3"){
    return(String(ip_3_int));
  }
  if(var == "ip_address_4"){
    return(String(ip_4_int));
  }
  return String();
}

void html_input(){
  server.begin();
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  if(netwerk){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      String http_zender = "                         ";
      String http_url = "                                                                             ";
      char terminator = char(0x0a);
      if(request->hasParam(KEUZEMIN_INPUT)){
        wachttijd = millis();
        web_kiezen = true;
        keuze--;
        while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
          keuze --;
        }
        if(keuze < -2){
          keuze = MAX_AANTAL_KANALEN - 1;
            while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
              keuze --;
            }
        }
      }
      if(request->hasParam(KEUZEPLUS_INPUT)){
        wachttijd = millis();
        web_kiezen = true;
        keuze++;
        if(keuze > MAX_AANTAL_KANALEN + 1){
          keuze = 0;
        }
        if((keuze > 0) && (keuze < MAX_AANTAL_KANALEN)){
          while((urlarray[keuze][0] != *h_char) && (keuze < MAX_AANTAL_KANALEN)){
            keuze ++;
          }
        }
        if(keuze == MAX_AANTAL_KANALEN){
          keuze = -2;
        }
        if(keuze == MAX_AANTAL_KANALEN + 1 ){
          keuze = -1;
        }
      }
      if(request->hasParam(BEVESTIGKEUZE_INPUT)){
        gekozen_bool = true;
        web_kiezen = false;
        streamsong = "";
        if(keuze == -2){
          songlijst_bestaat_bool = false;
          testDir(SD, "/songlijst0");
          if(songlijst_bestaat_bool == false){
            lijst_maken = true;
          }
          else{
            keuze = pref.getShort("station");
            webradio = true;
          }
          
        }
        else if(keuze == -1){
          gekozen = keuze;
          speel_mp3 = true;
        }
        else{
          gekozen = keuze;
          webradio = true;
        }
      }
      if(request->hasParam(LAAG)){
        laag_keuze = ((request->getParam(LAAG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(MIDDEN)){
        midden_keuze = ((request->getParam(MIDDEN)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(HOOG)){
        hoog_keuze = ((request->getParam(HOOG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME)){
        volume_keuze = ((request->getParam(VOLUME)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME_BEVESTIG)){
        if((laag_keuze > -41) && (laag_keuze < 7)){
          laag_gekozen = laag_keuze;
          }
          if((midden_keuze > -41) && (midden_keuze < 7)){
            midden_gekozen = midden_keuze;
          }
          if((hoog_keuze > -41) && (hoog_keuze < 7)){
            hoog_gekozen = hoog_keuze;
          }
          if((volume_keuze > -1) && (volume_keuze < 22)){
            volume_gekozen = volume_keuze;
          }
          audio.setVolume(volume_gekozen);
          audio.setTone(laag_gekozen, midden_gekozen, hoog_gekozen);
          pref.putShort("laag", laag_gekozen);
          pref.putShort("midden", midden_gekozen);
          pref.putShort("hoog", hoog_gekozen);
          pref.putShort("volume", volume_gekozen);
        }
      if(request->hasParam(ARRAY_MIN)){
        array_index -= 1;
        if(array_index < 0){
          array_index = MAX_AANTAL_KANALEN - 1;
        }
      }
      if(request->hasParam(ARRAY_PLUS)){
        array_index += 1;
        if(array_index > MAX_AANTAL_KANALEN - 1){
          array_index = 0;
        }
      }
      if(request->hasParam(ZENDER)){
        http_zender = (request->getParam(ZENDER)->value());
      }
      if(request->hasParam(URL)){
        http_url = (request->getParam(URL)->value());
      }
      if(request->hasParam(BEVESTIG_ZENDER)){
        memset(zenderarray[array_index], 0, sizeof(zenderarray[array_index]));
        http_zender.toCharArray(zenderarray[array_index], http_zender.length() + 1);
        memset(urlarray[array_index], 0, sizeof(urlarray[array_index]));
        http_url.toCharArray(urlarray[array_index], http_url.length() + 1);
        schrijf_csv = true; 
      }
    });
  }
  if(!netwerk){
    Serial.println("geen netwerk");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", netwerk_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      String netwerk = "                         ";
      String paswoord = "                          ";
      char terminator = char(0x0a);
      if(request->hasParam(MIN_INPUT)){
        wachttijd = millis();
        web_kiezen = true;
        gn_keuze --;
        if(gn_keuze < 0){
          gn_keuze = 2;
        }
        gn_netwerk_station_schrijf_tft();
      }
      if(request->hasParam(PLUS_INPUT)){
        wachttijd = millis();
        web_kiezen = true;
        gn_keuze ++;
        if(gn_keuze > 2){
          gn_keuze = 0;
        }
      }
      if(request->hasParam(BEVESTIG_MP3)){
        web_kiezen = false;
        streamsong = "";
        if(gn_keuze == 0){
          audio.stopSong();
        }
        if(gn_keuze == 1){
          speel_mp3 = true;
        }
        if(gn_keuze == 2){
          songlijst_bestaat_bool = false;
          testDir(SD, "/songlijst0");
          if(songlijst_bestaat_bool == false){
            lijst_maken = true;
          }
          else{
            gn_keuze = 1;
            speel_mp3 = true;
            gn_keuze_vorig = gn_keuze;
          }
        }
      }
      if(request->hasParam(LAAG)){
        laag_keuze = ((request->getParam(LAAG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(MIDDEN)){
        midden_keuze = ((request->getParam(MIDDEN)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(HOOG)){
        hoog_keuze = ((request->getParam(HOOG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME)){
        volume_keuze = ((request->getParam(VOLUME)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME_BEVESTIG)){
        if((laag_keuze > -41) && (laag_keuze < 7)){
          laag_gekozen = laag_keuze;
        }
        if((midden_keuze > -41) && (midden_keuze < 7)){
          midden_gekozen = midden_keuze;
        }
        if((laag_keuze > -41) && (laag_keuze < 7)){
          hoog_gekozen = hoog_keuze;
        }
        if((volume_keuze > -1) && (laag_keuze < 22)){
          volume_gekozen = volume_keuze;
        }
        audio.setVolume(volume_gekozen);
        audio.setTone(laag_gekozen, midden_gekozen, hoog_gekozen);
        pref.putShort("laag", laag_gekozen);
        pref.putShort("midden", midden_gekozen);
        pref.putShort("hoog", hoog_gekozen);
        pref.putShort("volume", volume_gekozen);
      }
      if(request->hasParam(STA_SSID)){
        netwerk = (request->getParam(STA_SSID)->value());
        netwerk = netwerk + String(terminator);
        netwerk.toCharArray(ssid, (netwerk.length() +1));
        writeFile(SD, "/ssid", ssid);
        ssid_ingevuld = true;
      }
      if(request->hasParam(STA_PSWD)){
        paswoord = (request->getParam(STA_PSWD)->value());
        paswoord = paswoord + String(terminator);
        paswoord.toCharArray(password, (paswoord.length() + 1));
        writeFile(SD, "/pswd", password);
        pswd_ingevuld = true;
      }
      if(request->hasParam(IP_1_KEUZE)){
        ip_1_string = (request->getParam(IP_1_KEUZE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_2_KEUZE)){
        ip_2_string = (request->getParam(IP_2_KEUZE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_3_KEUZE)){
        ip_3_string = (request->getParam(IP_3_KEUZE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_4_KEUZE)){
        ip_4_string = (request->getParam(IP_4_KEUZE)->value()) +String(terminator);
      }
      
      if((ssid_ingevuld) && (pswd_ingevuld)){
        ssid_ingevuld = false;
        pswd_ingevuld = false;
        ip_string = ip_1_string + "." + ip_2_string + "." + ip_3_string + "." + ip_4_string;
        ip_string.toCharArray(ip_char, (ip_string.length() + 1));
        writeFile(SD, "/ip", ip_char);
        Serial.println("Restart over 5 seconden");
        delay(5000);
        ESP.restart();
      }
    });
  }
}

void setup(){
  delay(2500);
  Serial.begin(115200);
  pinMode(STATION_A, INPUT_PULLUP);
  pinMode(STATION_B, INPUT_PULLUP);
  pinMode(STATION_OK, INPUT_PULLDOWN);
  pinMode(VOLUME_A, INPUT_PULLUP);
  pinMode(VOLUME_B, INPUT_PULLUP);
  pinMode(TOUCH_INT, INPUT_PULLUP);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  tft.init();
  touch_calibrate();
  if(!SD.begin()){
    Serial.println("check SD kaart");
  }
  pref.begin("WebRadio", false); 
  if(pref.getString("controle") != "dummy geladen"){
    pref.putShort("station", 1);
    pref.putShort("volume", 10);
    pref.putShort("laag", 0);
    pref.putShort("midden", 0);
    pref.putShort("hoog", 0);
    pref.putShort("files", mp3_per_songlijst);
    pref.putString("controle", "dummy geladen");
  }
  gekozen = pref.getShort("station");
  volume_gekozen = pref.getShort("volume");
  volume_keuze = volume_gekozen;
  laag_gekozen = pref.getShort("laag");
  laag_keuze = laag_gekozen;
  midden_gekozen = pref.getShort("midden");
  midden_keuze = midden_gekozen;
  hoog_gekozen = pref.getShort("hoog");
  hoog_keuze = hoog_gekozen;
  mp3_per_songlijst = pref.getShort("files");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume_gekozen);
  audio.setTone(laag_gekozen, midden_gekozen, hoog_gekozen);
  lees_CSV();
  readFile(SD, "/ssid");
  inputString.toCharArray(ssid, teller);
  readFile(SD, "/pswd");
  inputString.toCharArray(password, teller);
  readIP(SD, "/ip");
  SPI.end();
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GOLD);
  tft.setTextFont(4);
  tft.setCursor (80, 50);
  tft.print("Internet radio");
  tft.setCursor (90, 90);
  tft.print("mp3 speler");
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(1);
  tft.setCursor (0, 220);
  tft.print("thieu-b55   maart 2023");
  SPI.end();
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(2500);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP(APssid, APpswd);
  netwerk = true;
  wacht_op_netwerk = millis();
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    if(millis() - wacht_op_netwerk > 15000){
      netwerk = false;
      break;
    }
  }
  if(netwerk){  
    IPAddress subnet(WiFi.subnetMask());
    IPAddress gateway(WiFi.gatewayIP());
    IPAddress dns(WiFi.dnsIP(0));
    IPAddress static_ip(ip_1_int, ip_2_int, ip_3_int, ip_4_int);
    WiFi.disconnect();
    if (WiFi.config(static_ip, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    wacht_op_netwerk = millis();
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      if(millis() - wacht_op_netwerk > 15000){
        netwerk = false;
        break;
      }
    }
    keuze = gekozen;
    keuze_vorig = keuze;
    radio_gekozen();
    SPI.end();
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_SKYBLUE);
    tft.setTextFont(2);
    tft.setCursor((320 - (String(zenderarray[gekozen]).length() * 7)) / 2, tft_positie_int);
    tft.print(zenderarray[gekozen]);
    tft.setTextColor(TFT_GOLD);
    tft.setCursor((320 - (streamsong.length() * 7)) / 2, tft_positie_int + 20);
    tft.print(streamsong);
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    tft_update_long = millis();
    touch_update_long = millis();
    html_input();
  }
  else{
    gn_keuze_vorig = gn_keuze;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APssid, APpswd);
    SPI.end();
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextFont(2);
    tft.setCursor(115, 20);
    tft.print("GEEN INTERNET");
    tft.setCursor(94, 40);
    tft.print("MAAK VERBINDING MET");
    tft.setCursor(129, 80);
    tft.setTextColor(TFT_YELLOW);
    tft.print("NETWERK :");
    tft.setCursor(115, 100);
    tft.setTextColor(TFT_GREEN);
    tft.print("ESP32webradio");
    tft.setCursor(125, 120);
    tft.setTextColor(TFT_YELLOW);
    tft.print("PASWOORD :");
    tft.setCursor(128, 140);
    tft.setTextColor(TFT_GREEN);
    tft.print("ESP32pswd");
    tft.setCursor(146, 160);
    tft.setTextColor(TFT_YELLOW);
    tft.print("IP :");
    tft.setCursor(121, 180);
    tft.setTextColor(TFT_GREEN);
    tft.print("192.168.4.1");
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    html_input();
    tft_bericht_long = millis();
    while((millis() - tft_bericht_long) < 10000){
      yield();
    }
  }
  wachttijd = millis();
  attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touch, FALLING);
  attachInterrupt(digitalPinToInterrupt(STATION_A), station_a_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(STATION_B), station_b_int, CHANGE);
  attachInterrupt(digitalPinToInterrupt(VOLUME_A), volume_a_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(VOLUME_B), volume_b_int, CHANGE);
}

void loop(){
  /*
   * Rotary encoder station
   */
  if(netwerk){
    if(station_plus_bool){
      station_plus_bool = false;
      wachttijd = millis();
      kiezen = true;
      netwerk_station_wis_tft();
      keuze++;
      if(keuze > MAX_AANTAL_KANALEN + 1){
        keuze = 0;
      }
      if((keuze > 0) && (keuze < MAX_AANTAL_KANALEN)){
        while((urlarray[keuze][0] != *h_char) && (keuze < MAX_AANTAL_KANALEN)){
          keuze ++;
        }
      }
      if(keuze == MAX_AANTAL_KANALEN){
        keuze = -2;
      }
      if(keuze == MAX_AANTAL_KANALEN + 1 ){
        keuze = -1;
      }
      netwerk_station_schrijf_tft();
    }
    if(station_min_bool){
      station_min_bool = false;
      wachttijd = millis();
      kiezen = true;
      netwerk_station_wis_tft();
      keuze--;
      while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
        keuze --;
      }
      if(keuze < -2){
        keuze = MAX_AANTAL_KANALEN - 1;
          while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
            keuze --;
          }
      }
      netwerk_station_schrijf_tft();
    }
    if(digitalRead(STATION_OK)){
      kiezen = false;
      gekozen_bool = true;
      eerste_bool = false;
      streamsong = "";
      if(keuze == -2){
        songlijst_bestaat_bool = false;
        testDir(SD, "/songlijst0");
        if(songlijst_bestaat_bool == false){
          lijst_maken = true;
        }
        else{
          keuze = pref.getShort("station");
          webradio = true;
        }
      }
      else if(keuze == -1){
        gekozen = keuze;
        speel_mp3 = true;
      }
      else{
        gekozen = keuze;
        pref.putShort("station", gekozen);
        webradio = true;
      }
    } 
  }
  if(!netwerk){
    if(station_plus_bool){
      station_plus_bool = false;
      wachttijd = millis();
      kiezen = true;
      gn_netwerk_station_wis_tft();
      gn_keuze ++;
      if(gn_keuze > 2){
        gn_keuze = 0;
      }
      gn_netwerk_station_schrijf_tft();
    }
    if(station_min_bool){
      station_min_bool = false;
      wachttijd = millis();
      kiezen = true;
      gn_netwerk_station_wis_tft();
      gn_keuze --;
      if(gn_keuze < 0){
        gn_keuze = 2;
      }
      gn_netwerk_station_schrijf_tft();
    }
    if(digitalRead(STATION_OK)){
      kiezen = false;
      gekozen_bool = true;
      eerste_bool = false;
      streamsong = "";
      if(gn_keuze == 0){
        audio.stopSong();
      }
      if(gn_keuze == 1){
        speel_mp3 = true;
      }
      if(gn_keuze == 2){
        songlijst_bestaat_bool = false;
        testDir(SD, "/songlijst0");
        if(songlijst_bestaat_bool == false){
          lijst_maken = true;
        }
        else{
          gn_keuze = 1;
          speel_mp3 = true;
          gn_keuze_vorig = gn_keuze;
        }
      }
    }
  }
  /*
   * Rotary encoder Volume
   */
  if(volume_plus_bool){
    volume_plus_bool = false;
    wachttijd = millis();
    kiezen = true;
    SPI.end();
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);         // SPI TFT  
    tft.setTextFont(4);
    tft.setCursor(280, 116);
    tft.setTextColor(TFT_BLACK);
    tft.print(volume_gekozen);
    if(volume_gekozen < 21){
      volume_gekozen++;
    }
    volume_bewaren_bool = true;
    audio.setVolume(volume_gekozen);
    tft.setCursor(280, 116);
    tft.setTextColor(TFT_SKYBLUE);
    tft.print(volume_gekozen);
    tft.setTextFont(2);
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);           // SPI SD kaart
  }
  if(volume_min_bool){
    volume_min_bool = false;
    wachttijd = millis();
    kiezen = true;
    SPI.end();
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);         // SPI TFT  
    tft.setTextFont(4);
    tft.setCursor(280, 116);
    tft.setTextColor(TFT_BLACK);
    tft.print(volume_gekozen);
    if(volume_gekozen > 0){
      volume_gekozen--;
    }
    volume_bewaren_bool = true;
    audio.setVolume(volume_gekozen);
    tft.setCursor(280, 116);
    tft.setTextColor(TFT_SKYBLUE);
    tft.print(volume_gekozen);
    tft.setTextFont(2);
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);           // SPI SD kaart
  }
  /*
   * Inlezen touchscreen
   */
  if(touch_int_bool){
    t_x = 0;
    t_y = 0; 
    SPI.end();                                              
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TOUCH_CS);       // SPI touchscreen
    int max_loop = 0;
    pressed = tft.getTouch(&t_x, &t_y);
    while((!pressed) && (max_loop < 10)){
      max_loop++;
      pressed = tft.getTouch(&t_x, &t_y);
    }
    wachttijd = millis();
    kiezen = true;
    touch_int_bool = false;
    int_bool = true;
    touch_update_long = millis();
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);             // SPI SD kaart
    
    Serial.print("x :");
    Serial.println(t_x);
    Serial.print("y :");
    Serial.println(t_y);
    
    if(eerste_bool){ 
      SPI.end();
      SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);         // SPI TFT  
      /*
       * Geluid +
       */  
      if((t_x >= 1) && (t_x <= 50) && (t_y >= 1) && (t_y <= 50)){   
        tft.setTextFont(4);
        tft.setCursor(280, 116);
        tft.setTextColor(TFT_BLACK);
        tft.print(volume_gekozen);
        tft.setCursor(280, 116);
        tft.setTextColor(TFT_SKYBLUE);
        if(volume_gekozen < 21){
          volume_gekozen++;
        }
        tft.print(volume_gekozen);
        pref.putShort("volume", volume_gekozen);
        audio.setVolume(volume_gekozen);
        tft.setTextFont(2);
      }
      /*
       * Geluid -
       */
      if((t_x >= 270) && (t_x <= 320) && (t_y >= 1) && (t_y <= 50)){
        tft.setTextFont(4);
        tft.setCursor(280, 116);
        tft.setTextColor(TFT_BLACK);
        tft.print(volume_gekozen);
        tft.setCursor(280, 116);
        tft.setTextColor(TFT_SKYBLUE);
        if(volume_gekozen > 0){
          volume_gekozen--;
        }
        tft.print(volume_gekozen);
        pref.putShort("volume", volume_gekozen);
        audio.setVolume(volume_gekozen);
        tft.setTextFont(2);
      }
      SPI.end();
      SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);           // SPI SD kaart
      if(netwerk){
        /*
         * Preset +
         */
        if((t_x >= 1) && (t_x <= 50) && (t_y >= 190) && (t_y <= 240)){
          netwerk_station_wis_tft();
          keuze++;
          if(keuze > MAX_AANTAL_KANALEN + 1){
            keuze = 0;
          }
          if((keuze > 0) && (keuze < MAX_AANTAL_KANALEN)){
            while((urlarray[keuze][0] != *h_char) && (keuze < MAX_AANTAL_KANALEN)){
              keuze ++;
            }
          }
          if(keuze == MAX_AANTAL_KANALEN){
            keuze = -2;
          }
          if(keuze == MAX_AANTAL_KANALEN + 1 ){
            keuze = -1;
          }
          netwerk_station_schrijf_tft();
        }
        /*
         * Kies 
         */
        if((t_x >= 135) && (t_x <= 185) && (t_y >=190) && (t_y <= 240)){
          kiezen = false;
          gekozen_bool = true;
          eerste_bool = false;
          streamsong = "";
          if(keuze == -2){
            songlijst_bestaat_bool = false;
            testDir(SD, "/songlijst0");
            if(songlijst_bestaat_bool == false){
              lijst_maken = true;
            }
            else{
              keuze = pref.getShort("station");
              webradio = true;
            }
          }
          else if(keuze == -1){
            gekozen = keuze;
            speel_mp3 = true;
          }
          else{
            gekozen = keuze;
            pref.putShort("station", gekozen);
            webradio = true;
          }
        }
        /*
         * Preset -
         */
        if((t_x >= 280) && (t_x <= 310) && (t_y >= 200) && (t_y <= 230)){
          netwerk_station_wis_tft();
          keuze--;
          while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
            keuze --;
          }
          if(keuze < -2){
            keuze = MAX_AANTAL_KANALEN - 1;
              while((urlarray[keuze][0] != *h_char) && (keuze > 0)){
                keuze --;
              }
          }
          netwerk_station_schrijf_tft();
        }
      }
      if(!netwerk){
        /*
         * Preset +
         */
        if((t_x >= 1) && (t_x <= 50) && (t_y >= 190) && (t_y <= 240)){
          gn_netwerk_station_wis_tft();
          gn_keuze ++;
          if(gn_keuze > 2){
            gn_keuze = 0;
          }
          gn_netwerk_station_schrijf_tft();
        }
        /*
         * Kies
         */
        if((t_x >= 135) && (t_x <= 185) && (t_y >=190) && (t_y <= 240)){
          kiezen = false;
          gekozen_bool = true;
          eerste_bool = false;
          streamsong = "";
          if(gn_keuze == 0){
          audio.stopSong();
          }
          if(gn_keuze == 1){
            speel_mp3 = true;
          }
          if(gn_keuze == 2){
            songlijst_bestaat_bool = false;
            testDir(SD, "/songlijst0");
            if(songlijst_bestaat_bool == false){
              lijst_maken = true;
            }
            else{
              gn_keuze = 1;
              speel_mp3 = true;
              gn_keuze_vorig = gn_keuze;
            }
          }
        }
        /*  
         *   Preset -
         */
        if((t_x >= 280) && (t_x <= 310) && (t_y >= 200) && (t_y <= 230)){
          gn_netwerk_station_wis_tft();
          gn_keuze --;
          if(gn_keuze < 0){
            gn_keuze = 2;
          }
          gn_netwerk_station_schrijf_tft();
        }
      }
    }
  }
    
  if(((millis() - touch_update_long) > 500) && (int_bool)){
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), dummy, FALLING);
    detachInterrupt(digitalPinToInterrupt(TOUCH_INT));
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touch, FALLING);
    int_bool = false;
  }
  
  if(!kiezen){
    if(((millis() - tft_update_long) > 10000) || (gekozen_bool)){
      gekozen_bool = false;
      tft_update_long = millis();
      tft_positie_int += tft_positie_plus_int;
      if((tft_positie_int < 20) || (tft_positie_int > 170)){
        tft_positie_plus_int = tft_positie_plus_int * -1;
      }
      SPI.end();
      SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);           // SPI TFT
      if(netwerk){
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_SKYBLUE);
        tft.setTextFont(2);
        if(gekozen == - 1){
          tft.setCursor(121, tft_positie_int);
          tft.print("mp3 speler");
        }
        else if(gekozen == -2){
          tft.setCursor(114, tft_positie_int);
          tft.print("mp3 lijst maken");
        }
        else{
          tft.setCursor((320 - ((String(zenderarray[gekozen]).length() -1) * 7)) / 2, tft_positie_int);
          tft.print(zenderarray[gekozen]);
        }
        tft.setTextColor(TFT_GOLD);
        tft.setCursor((320 - ((streamsong.length() -1) * 7)) / 2, tft_positie_int + 20);
        tft.print(streamsong);
        tft.setCursor(0, tft_positie_int + 197);
        tft.print(WiFi.localIP());
      }
      if(!netwerk){
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_SKYBLUE);
        tft.setTextFont(2);
        if(gn_keuze == 0){
          tft.setCursor(113, tft_positie_int);
          tft.print("stop mp3 speler");
        }
        if(gn_keuze == 1){
          tft.setCursor(121, tft_positie_int);
          tft.print("mp3 speler");
        }
        if(gn_keuze == 2){
          tft.setCursor(114, tft_positie_int);
          tft.print("mp3 lijst maken");
        }
        tft.setTextColor(TFT_GOLD);
        tft.setCursor((320 - ((streamsong.length() -1) * 7)) / 2, tft_positie_int + 20);
        tft.print(streamsong);
      }
      SPI.end();
      SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);               // SPI SD kaart
    }
  }
  if(kiezen && !eerste_bool){
    eerste_bool = true;
    keuze_vorig = keuze;
    gn_keuze_vorig = gn_keuze;
    SPI.end();
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);             // SPI TFT
    tft.fillScreen(TFT_BLACK);
    tft.fillTriangle(25, 10, 10, 40, 40, 40, TFT_MAGENTA);
    tft.fillRoundRect(10, 105, 30, 30, 5, TFT_VIOLET);
    tft.fillTriangle(25, 230, 10, 200, 40, 200, TFT_MAGENTA);
    tft.fillTriangle(295, 10, 280, 40, 310, 40, TFT_YELLOW);
    tft.fillTriangle(295, 230, 280, 200, 310, 200, TFT_YELLOW);
    tft.setTextFont(2);
    tft.setTextColor(TFT_SKYBLUE);
    if(netwerk){
      if(keuze == - 1){
        tft.setCursor(121, 80);
        tft.print("mp3 speler");
      }
      else if(keuze == -2){
        tft.setCursor(114, 80);
        tft.print("mp3 lijst maken");
      }
      else{
        tft.setCursor((320 - (String(zenderarray[keuze]).length() * 7)) / 2, 80);
        tft.print(zenderarray[keuze]);
      }
    }
    if(!netwerk){
      if(gn_keuze == 0){
        tft.setCursor(113, 80);
        tft.print("stop mp3 speler");
      }
      if(gn_keuze == 1){
        tft.setCursor(121, 80);
        tft.print("mp3 speler");
      }
      if(gn_keuze == 2){
        tft.setCursor(114, 80);
        tft.print("mp3 lijst maken");
      }
    }
    tft.setTextFont(4);
    tft.setCursor(280, 116);
    tft.print(volume_gekozen);
    tft.setTextFont(2);
    SPI.end();
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);               // SPI SD kaart
  }
  
  
  if(schrijf_csv == true){
    schrijf_csv = false;
    schrijf_naar_csv();
  }
  if(lijst_maken == true){
    mp3_lijst_maken_gekozen();
  }
  if(speel_mp3 == true){
    speel_mp3 = false;
    mp3_gekozen();
  }
  if(webradio == true){
    webradio = false;
    gekozen = keuze;
    pref.putShort("station", gekozen);
    radio_gekozen();
  }
  if(((millis() - wachttijd) > 5000) && (kiezen || web_kiezen)){
    /*
     * reset interrupts indien geen respons
     */
    detachInterrupt(digitalPinToInterrupt(TOUCH_INT));
    detachInterrupt(digitalPinToInterrupt(STATION_A));
    detachInterrupt(digitalPinToInterrupt(VOLUME_A));
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touch, FALLING);
    attachInterrupt(digitalPinToInterrupt(STATION_A), station_a_int, FALLING);
    attachInterrupt(digitalPinToInterrupt(VOLUME_A), volume_a_int, FALLING);
    kiezen = false;
    web_kiezen = false;
    keuze = gekozen;
    eerste_bool = false;
    if(volume_bewaren_bool){
      volume_bewaren_bool = false;
      pref.putShort("volume", volume_gekozen);
    }
  }

   /*
   * debouncing rotary encoders
   */
  if(station_B_bool){
    int_begin_long = millis();
    while((millis() - int_begin_long) < int_wachttijd_long){
      audio.loop();
      yield();
    }
    attachInterrupt(digitalPinToInterrupt(STATION_A), dummy, CHANGE);
    detachInterrupt(digitalPinToInterrupt(STATION_A));
    attachInterrupt(digitalPinToInterrupt(STATION_B), dummy, CHANGE);
    detachInterrupt(digitalPinToInterrupt(STATION_B));
    station_B_bool = false;
    station_A_bool = false;
    attachInterrupt(digitalPinToInterrupt(STATION_A), station_a_int, FALLING);
    attachInterrupt(digitalPinToInterrupt(STATION_B), station_b_int, FALLING);
  }

  
  if(volume_B_bool){
    int_begin_long = millis();
    while((millis() - int_begin_long) < int_wachttijd_long){
      audio.loop();
      yield();
    }
    attachInterrupt(digitalPinToInterrupt(VOLUME_A), dummy, CHANGE);
    detachInterrupt(digitalPinToInterrupt(VOLUME_A));
    attachInterrupt(digitalPinToInterrupt(VOLUME_B), dummy, CHANGE);
    detachInterrupt(digitalPinToInterrupt(VOLUME_B));
    volume_B_bool = false;
    volume_A_bool = false;
    attachInterrupt(digitalPinToInterrupt(VOLUME_A), volume_a_int, FALLING);
    attachInterrupt(digitalPinToInterrupt(VOLUME_B), volume_b_int, FALLING);
  }
  
  audio.loop();
}
