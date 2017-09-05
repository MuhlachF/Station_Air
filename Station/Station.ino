/* 
 * A développer 
 * Utilisatoin du bargraphe
 * encadrement des niveaux
 */
/* 
 * Carbon monoxide CO 1 – 1000ppm
 * Nitrogen dioxide NO2 0.05 – 10ppm
 * Ethanol C2H6OH 10 – 500ppm
 * Hydrogen H2 1 – 1000ppm
 * Ammonia NH3 1 – 500ppm
 * Methane CH4 >1000ppm
 * Propane C3H8 >1000ppm
 * Iso-butane C4H10 >1000ppm
 *  
 *  DANGERS 
 *  CO2 - 400ppm en moyenne pour air extérieur
 *      - Agence Nationale de Sécurité Sanitaire de l'Alimentation de l'Environnmenent et du Travail : valeurs limites 1000 à 1500ppm
 *  CO1 - Monoxyde de Carbone exposition maximale :
 *      10 mg/m3 (10 ppm) pendant 8 heures.
 *      30 mg/m3 (25ppm) pendant 1 heure.
 *      60 mg/m3 (50ppm) pendant 30 min.
 *      100 mg/m3(90ppm) pendant 15 min.
 * NO2 - Dioxyde d'azote : 
 *      5 ppm - Seuil des effets réversibles 
 *      Seuils d'effets réversibles
 * NH3 AMMONIAC TEMPS (min) CONCENTRATION mg/m3 ppm - Seuil olfactif : 5-50 ppm
 *               1         196                 280
 *               3         140                 200
 *               10        105                 150
 *               20        84                  120
 *               30        77                  110
 *               60        56                  80
 * 
 *      
 *      Version 0.1
 *      Version 0.2 : si bouton PRECEDENT + SUIVANT SONT APPUYES AU DEMARRAGE -> LANCEMENT DES OPERATIONS DE CALIBRATION DES CAPTEURS
 *      Version 0.3 : Fonction de prechauffage
 *      Version 0.4 : passage Arduino Mega
 *      Version 0.5 Debug DHT11 / arduino MEGA
 *      
 *      A faire : pilotage ventilateur et mise en route après prechauffage
 *      Enregistrement des données sur carte microsd
 */
#define VERSION_MAJ 0
#define VERSION_MIN 5


/*
 * LIBRAIRIES
 */

#include <idDHT11.h>
#include <Wire.h>
#include "rgb_lcd.h"
//#include <SoftwareSerial.h>
#include "MutichannelGasSensor.h"
#include <Grove_LED_Bar.h>
#include <SPI.h>
#include <SD.h>


/*
 * DIRECTIVES PREPROCESSEUR
 */
#define SENSOR_ADDR     0X04       // Adresse I2C pour le Multi Gaz Sensor
#define BOUTON_SUIVANT 6           // Bouton de navigation suivant
#define BOUTON_PRECEDENT 7         // Bouton de navigation précédent
#define BOUTON_RECORD 8            // Permet l'enregistrement des données sur une carte micro_SD

#define MOTOR 13
#define COVSensor A1               // Capteur COV sur le port A1
#define O2Sensor A2                // Capteur O2 sur le port A2
#define BARGRAPH_CLOCK 5
#define BARGRAPH_DATA 4
#define LED_RECORD 9               // LED témoin enregistrement
#define CHIP_SELECT 10             // Selection carte SD sur bus SPI
#define CO2_TX 2
#define CO2_RX 3    
#define Vc 4.95                    // Référence pour la calibration du capteur COV
#define R 50                       // Référence R0 pour le capteur COV
#define VRefer 3.3                 // Référence pour le le capteur O2
#define PRE_HEAT_TIME   15         // Temps de préchauffage 15 min pour le Multi Gaz Sensor, 10-30 minutes is recommended
#define CO2_VALEUR_INF 400         // Niveau de CO2 moyen / air extérieur 400ppm
#define CO2_VALEUR_SUP 1250        // Seuil max à ne pas dépasser dans les locaux 1250ppm (1000 - 1500 ANSES)
#define HUM_VALEUR_SUP 100         // Seuil Maximal humidité 100% conditions normales 50%
#define MONOXYDE_VALEUR_SUP 10     // On considère qu'un taux de 10ppm de monoxyde de carbonne est dangeureux pour la santé
#define OXYGEN_VALEUR_MIN 1900     // En dessous de 19% d'oxygene dans l'air des troubles de la santé peuvent apparaître
#define OXYGEN_VALEUR_NORM 2100    // La concentration moyenne d'oxygene dans l'air est de 21%
#define DIOXYDE_AZOTE_VALEUR_SUP 5 // Seuil maximale de 5ppm - Seuil des effets réversibles 
#define DIHYDROGENE_VALEUR_SUP 5   // Seuil maximale de 5ppm 
#define AMMONIAC_VALEUR_SUP 50     // Seuil maximale de 50 ppm - Seuil olfactif : 5-50 ppm
#define GAZ_INFLAMMABLES 1000      // Seuil de detection 1000ppm
#define ETHANOL_VALEUR_SUP 100     // Seuil maximal de détection de l'Ethanol
#define COV_VALEUR_SUP 10          // irritation légère des yeux à partir de 1 ppm et des voix respiratoires à partir de 2ppm
#define MAX_MESURES 11             
#define TIMER 500
#define RECORD_TIMER 60            // 1 enregistrement par minute


enum Mesures {Temperature,CO2,O2,Monoxyde,Dioxyde,Ammoniac,Propane,Butane,Methane,Dihydrogene,Ethanol,COV};
enum Mesures mesure = Temperature;
int selection = 0;
int selectionBack = 0;

/* 
 * HEADER 
 */
void dht11_wrapper(); // must be declared before the lib initialization
float readO2Vout();                // Lecture des mesures du capteur O2
float readConcentration();         // Donne le pourcentage de O2 dans l'air
void AffichageTempHum();           // Affichage des  informations du capteur DHT11
void AffichageCO2();               // Affichage du niveau de CO2 en ppm
void AffichageO2();                // Affichage du pourcentage de O2 dans l'air
void AffichageMonoxyde();          // Affichage du niveau de CO en ppm
void AffichageDioxydeAzote();
void AffichageAmmoniac();
void AffichagePropane();
void AffichageButane();
void AffichageMethane();
void AffichageDihydrogene();
void AffichageEthanol();
void calibrationCOV();             // Fonction de calibration du capteur COV R0 recherché 50 Ohms
void calibrationMulti();           // Fonction de calibration du capteur Multi Gas
void Bargraph(int limiteInf, int limiteSup, int valeur);
void Prechauffage();



/*
 * DECLARATION DES CONSTANTES
 */
const unsigned char cmd_get_sensor[] =  // Tableau de constante pour le capteur CO2
{
    0xff, 0x01, 0x86, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79
};

/* 
 * DECLARATION DES VARIABLES 
 */
int idDHT11pin = 2; //Digital pin for comunications
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
unsigned char dataRevice[9];      // Tableau de valeurs pour le capteur CO2
int CO2PPM;                       // Valeur de CO2
float Vout =0;                    // Valeur / capteur O2
bool etatBoutonG;
bool etatBoutonD;
bool etatBoutonRecord;
bool record = false;
long tempsEcoule = 0;
long tempsBack = 0;
bool checkCard;

/*
 * ECRAN LCD
 */
rgb_lcd lcd;

/*
 * DHT11
 */
idDHT11 DHT11(idDHT11pin,idDHT11intNumber,dht11_wrapper);
 
//File myFile;

/*
 * Configuration Capteur CO2 en liaison série
 */
//SoftwareSerial s_serial(CO2_TX,CO2_RX);     // Configuration d'une liaison série sur les ports 2 et 3 TX RX pour le capteur CO2
#define sensor Serial1            // Capteur CO2 sur le port série 2-3

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

/*
 * BARGRAPH
 */
Grove_LED_Bar bar(BARGRAPH_CLOCK,BARGRAPH_DATA, 1);  // Clock pin, Data pin, Orientation

void setup() 
{
    pinMode (BOUTON_SUIVANT, INPUT);
    pinMode (BOUTON_PRECEDENT, INPUT);
    pinMode (BOUTON_RECORD,INPUT);
    pinMode (LED_RECORD, OUTPUT);

    pinMode (MOTOR,OUTPUT);
    digitalWrite (MOTOR,LOW);

    checkCard = SD.begin(CHIP_SELECT); 
    delay (500);
    
    lcd.begin(16, 2);                 // Configuration de l'écran LCD 2 lignes de 16 caractères
    lcd.setRGB(colorR, colorG, colorB);

    if (!checkCard)     // Vérification de létat de la carte SD
    {
      lcd.print("PROBLEME CARTE SD");
      delay(1000);
    }
    else
    {
      lcd.print ("CARTE SD OK");
      delay (1000);
    }
    bar.begin();                      // Initialisation du barpgraph
    
    sensor.begin(9600);               // Configuration de la liaison série du capteur CO2
    gas.begin(SENSOR_ADDR);           // Initialisation de la connexion I2C avec le Multi Gaz Sensor 
    Serial.begin(9600);

    bool boutonInit = (digitalRead(BOUTON_SUIVANT)&&digitalRead(BOUTON_PRECEDENT));
    if (boutonInit)
    {
      calibrationCOV();
      CalibrationMulti();
    }
    Prechauffage();
    digitalWrite(MOTOR,HIGH);
}
// This wrapper is in charge of calling 
// mus be defined like this for the lib work
void dht11_wrapper() {
  DHT11.isrCallback();
}

void loop() 
{
     tempsEcoule = millis();
     etatBoutonD = digitalRead(BOUTON_SUIVANT);
     etatBoutonG = digitalRead(BOUTON_PRECEDENT);
     etatBoutonRecord = digitalRead(BOUTON_RECORD);
     
     dataRecieve();
     
     if (etatBoutonG)
     {
        delay(50);
        if (mesure < MAX_MESURES) mesure = mesure + 1;
        delay(200);        
     }
     else if (etatBoutonD)
     {
        delay(50);
        if (mesure > 0 ) mesure = mesure - 1;
        delay(200);        
     }
     else if (etatBoutonRecord)
     {
        delay(50);
        record = !record;
        delay(200);
     }

     if (record)
     {
        lcd.clear();
        lcd.print("Enregistrement...");
        digitalWrite(LED_RECORD,HIGH);
        
        /*
        myFile = SD.open("Data.flv", FILE_WRITE);

        if (myFile) 
        {
          Serial.print("Writing to test.txt...");
          myFile.println("testing 1, 2, 3.");
    
          myFile.close();
          Serial.println("done.");
        }
        else 
        {
            Serial.println("error opening test.txt");
        }
        */
        delay (RECORD_TIMER);
     }
     else
     {
      digitalWrite(LED_RECORD,LOW);
      if ((tempsEcoule - tempsBack) > TIMER)
     {
          switch (mesure)
          {
            case Temperature:
            AffichageTempHum();
            break;
      
            case CO2:
            AffichageCO2();
            break;

            case Monoxyde:
            AffichageMonoxyde();
            break;

            case O2:
            AffichageO2();
            break;

            case Dioxyde:
            AffichageDioxydeAzote();
            break;

            case Dihydrogene:
            AffichageDihydrogene();
            break;

            case Ammoniac:
            AffichageAmmoniac();
            break;

            case Propane:
            AffichagePropane();
            break;

            case Butane:
            AffichageButane();
            break;

            case Methane:
            AffichageMethane();
            break;

            case Ethanol:
            AffichageEthanol();
            break;

            case COV:
            AffichageCOV();
            break;

          }
          tempsBack = tempsEcoule;
     }
     }
     
}

/* ----------------------------------------------------------------------------------*/

/***************************************************/
/* FONCTION AFFICHAGE TAUX HUMIDITE ET TEMPERATURE */
/***************************************************/
void AffichageTempHum()
{
   DHT11.acquire();
  while (DHT11.acquiring())
    ;
  int result = DHT11.getStatus();
  switch (result)
  {
  case IDDHTLIB_ERROR_CHECKSUM: 
    Serial.println("Error\n\r\tChecksum error"); 
    break;
  case IDDHTLIB_ERROR_ISR_TIMEOUT: 
    Serial.println("Error\n\r\tISR Time out error"); 
    break;
  case IDDHTLIB_ERROR_RESPONSE_TIMEOUT: 
    Serial.println("Error\n\r\tResponse time out error"); 
    break;
  case IDDHTLIB_ERROR_DATA_TIMEOUT: 
    Serial.println("Error\n\r\tData time out error"); 
    break;
  case IDDHTLIB_ERROR_ACQUIRING: 
    Serial.println("Error\n\r\tAcquiring"); 
    break;
  case IDDHTLIB_ERROR_DELTA: 
    Serial.println("Error\n\r\tDelta time to small"); 
    break;
  case IDDHTLIB_ERROR_NOTSTARTED: 
    Serial.println("Error\n\r\tNot started"); 
    break;
  default: 
    Serial.println("Unknown error"); 
    break;
  }

  float humidite = DHT11.getHumidity();
  float temperature = DHT11.getCelsius();
  
  lcd.clear();
  lcd.print("Humidite:"+String (humidite,0)+"%");
  lcd.setCursor(0,1);
  lcd.print("Temperature:"+String (temperature,0)+"C");
  Bargraph(0,HUM_VALEUR_SUP,humidite);
}
/*******************************************************/
/* FIN FONCTION AFFICHAGE TAUX HUMIDITE ET TEMPERATURE */
/*******************************************************/

/* ----------------------------------------------------------------------------------*/


/****************************************/

/* ----------------------------------------------------------------------------------*/

/**********************************************************/
/* FONCTION RECEPTION INFOS CAPTEUR CO2 VIA LIAISON SERIE */
/**********************************************************/
bool dataRecieve(void)
{
    byte data[9];
    int i = 0;

    //transmit command data
    for(i=0; i<sizeof(cmd_get_sensor); i++)
    {
        sensor.write(cmd_get_sensor[i]);
    }
    delay(10);
    //begin reveiceing data
    if(sensor.available())
    {
        while(sensor.available())
        {
            for(int i=0;i<9; i++)
            {
                data[i] = sensor.read();
            }
        }
    }

    for(int j=0; j<9; j++)
    {
        Serial.print(data[j]);
        Serial.print(" ");
    }
    Serial.println("");

    if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]))) != data[8])
    {
        return false;
    }

    CO2PPM = (int)data[2] * 256 + (int)data[3];

    return true;
}
/**************************************************************/
/* FIN FONCTION RECEPTION INFOS CAPTEUR CO2 VIA LIAISON SERIE */
/**************************************************************/

/**************************/
/* FONCTION AFFICHAGE CO2 */
/**************************/
void AffichageCO2()
{
  lcd.clear();
  lcd.print("CO2 : "+String(CO2PPM)+" ppm");
  Bargraph(CO2_VALEUR_INF,CO2_VALEUR_SUP,CO2PPM);
}
/******************************/
/* FIN FONCTION AFFICHAGE CO2 */
/******************************/

/* ----------------------------------------------------------------------------------*/

/**********************/
/* FONCTION MESURE O2 */
/**********************/
float readO2Vout()
{
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(O2Sensor);
    }

    sum >>= 5;

    float MeasuredVout = sum * (VRefer / 1023.0);
    return MeasuredVout;
}

/**************************/
/* FIN FONCTION MESURE O2 */
/**************************/

/* ----------------------------------------------------------------------------------*/

/**************************************/
/* FONCTION POURCENTAGE O2 DANS L'AIR */
/**************************************/
float readConcentration()
{
    // Vout samples are with reference to 3.3V
    float MeasuredVout = readO2Vout();

    //float Concentration = FmultiMap(MeasuredVout, VoutArray,O2ConArray, 6);
    //when its output voltage is 2.0V,
    float Concentration = MeasuredVout * 0.21 / 2.0;
    float Concentration_Percentage=Concentration*100;
    return Concentration_Percentage;
}
/******************************************/
/* FIN FONCTION POURCENTAGE O2 DANS L'AIR */
/******************************************/

/* ----------------------------------------------------------------------------------*/

/************************************/
/* FONCTION AFFICHAGE O2 DANS L'AIR */
/************************************/

void AffichageO2()
{
  float ConcentrationO2 = readConcentration();
  lcd.clear();
  lcd.print("Concentration O2");
  lcd.setCursor(0,1);
  lcd.print(String(ConcentrationO2)+"%");
  int ConcentrationToInt = int(float(ConcentrationO2*100));
  Bargraph2(OXYGEN_VALEUR_MIN,OXYGEN_VALEUR_NORM,ConcentrationToInt);
}
/****************************************/
/* FIN FONCTION AFFICHAGE O2 DANS L'AIR */
/****************************************/

/* ----------------------------------------------------------------------------------*/

/*******************************/
/* FONCTION DE CALIBRATION COV */
/*******************************/
void calibrationCOV()
{
   bool boutonInit = false;
   lcd.clear();
   lcd.print("CALIBRATION COV");
   delay(2000);
  do
  { 
   boutonInit = (digitalRead(BOUTON_SUIVANT)&&digitalRead(BOUTON_PRECEDENT));
   
   int sensorValue=analogRead(A1);
   float R0=(1023.0/sensorValue)-1;
   lcd.clear();
   lcd.print("R0 -> 50 Ohms");
   lcd.setCursor(0,1);
   lcd.print("Valeur R0:"+String(R0)+" Ohms");
   delay(200);
    
  }while (!boutonInit);
   
}
/***********************************/
/* FIN FONCTION DE CALIBRATION COV */
/***********************************/

/* ----------------------------------------------------------------------------------*/

/********************************************/
/* FONCTION DE CALIBRATION DU CAPTEUR MULTI */
/********************************************/
void CalibrationMulti()
{
   bool boutonInit = false;
   lcd.clear();
   lcd.print("CALIBR MULTI");
   delay(2000);

   boutonInit = (digitalRead(BOUTON_SUIVANT)&&digitalRead(BOUTON_PRECEDENT));

  if (boutonInit)
  {
    Prechauffage();
    lcd.clear();
    lcd.print("CALIBRATION...");
    gas.doCalibrate();
    lcd.setCursor(0,1);
    lcd.print("SONDE OK");
    delay (2000);
  }
}
/************************************************/
/* FIN FONCTION DE CALIBRATION DU CAPTEUR MULTI */
/************************************************/


/**************************/
/* FONCTION AFFICHAGE COV */
/**************************/
void AffichageCOV()
{
    int sensorValue=analogRead(COVSensor);
    double Rs=(1023.0/sensorValue)-1;
    double ppm=pow(10.0,((log10(Rs/R)-0.0827)/(-0.4807)));
    lcd.clear();
    lcd.print("HCHO : "+String(ppm)+" ppm"); 
    Bargraph(0,COV_VALEUR_SUP,ppm);
}
/******************************/
/* FIN FONCTION AFFICHAGE COV */
/******************************/

/* ----------------------------------------------------------------------------------*/

/*********************************************/
/* FONCTION AFFICHAGE MONOXYDE DE CARBONE CO1*/
/*********************************************/
void AffichageMonoxyde()
{
    float c;
    c = gas.measure_CO();
    lcd.clear();
    lcd.print("Monoxyde CO");
    lcd.setCursor(0,1);
    if(c>=0) lcd.print(String(c)+" ppm");
    else lcd.print("Neant");
    Bargraph(0,MONOXYDE_VALEUR_SUP,c);
}
/*************************************************/
/* FIN FONCTION AFFICHAGE MONOXYDE DE CARBONE CO1*/
/*************************************************/

/* ----------------------------------------------------------------------------------*/

/*****************************************/
/* FONCTION AFFICHAGE DIOXYDE d'AZOTE NO2*/
/*****************************************/
void AffichageDioxydeAzote()
{
    float c;
    c = gas.measure_NO2();
    lcd.clear();
    lcd.print("Dioxyde Azte NO2");
    lcd.setCursor(0,1);
    if(c>=0) lcd.print(String(c)+" ppm");
    else lcd.print("Neant");
    int ConcentrationToInt = int(float(c*10));
    Bargraph(0,DIOXYDE_AZOTE_VALEUR_SUP*10,ConcentrationToInt);
}
/*********************************************/
/* FIN FONCTION AFFICHAGE DIOXYDE d'AZOTE NO2*/
/*********************************************/

/* ----------------------------------------------------------------------------------*/

/***********************************/
/* FONCTION AFFICHAGE AMMONIAC NH3 */
/***********************************/
void AffichageAmmoniac()
{
    float c;
    c = gas.measure_NH3();
    lcd.clear();
    lcd.print("Ammoniac NH3");
    lcd.setCursor(0,1);
    if(c>=0) lcd.print(String(c)+" ppm");
    else lcd.print("Neant");
    Bargraph(0,AMMONIAC_VALEUR_SUP,c);
}
/***************************************/
/* FIN FONCTION AFFICHAGE AMMONIAC NH3 */
/***************************************/

/* ----------------------------------------------------------------------------------*/

/***********************************/
/* FONCTION AFFICHAGE PROPANE C3H8 */
/***********************************/
void AffichagePropane()
{
    float c;
    c = gas.measure_C3H8();
    lcd.clear();
    lcd.print("Propane C3H8");
    lcd.setCursor(0,1);
    if(c>=1000)
    {
      lcd.print(String(c)+" ppm");
      bar.setLevel(10);
    }
    else
    {
      lcd.print("Gaz non detecte");
      bar.setLevel(0);
    }
}
/***************************************/
/* FIN FONCTION AFFICHAGE PROPANE C3H8 */
/***************************************/

/* ----------------------------------------------------------------------------------*/

/***********************************/
/* FONCTION AFFICHAGE BUTANE C4H10 */
/***********************************/
void AffichageButane()
{
    float c;
    c = gas.measure_C4H10();
    lcd.clear();
    lcd.print("Butane C4H10");
    lcd.setCursor(0,1);
    if(c>=1000)
    {
      lcd.print(String(c)+" ppm");
      bar.setLevel(10);
    }
    else
    {
      lcd.print("Gaz non detecte");
      bar.setLevel(0);
    }
}
/***************************************/
/* FIN FONCTION AFFICHAGE BUTANE C4H10 */
/***************************************/

/* ----------------------------------------------------------------------------------*/

/**********************************/
/* FONCTION AFFICHAGE METHANE CH4 */
/**********************************/
void AffichageMethane()
{
    float c;
    c = gas.measure_CH4();
    lcd.clear();
    lcd.print("Methane CH4");
    lcd.setCursor(0,1);
    if(c>=1000)
    {
      lcd.print(String(c)+" ppm");
      bar.setLevel(10);
    }
    else
    {
      lcd.print("Gaz non detecte");
      bar.setLevel(0);
    }
}
/**************************************/
/* FIN FONCTION AFFICHAGE METHANE CH4 */
/**************************************/

/* ----------------------------------------------------------------------------------*/

/*************************************/
/* FONCTION AFFICHAGE DIHYDROGENE H2 */
/*************************************/
void AffichageDihydrogene()
{
    float c;
    c = gas.measure_H2();
    lcd.clear();
    lcd.print("Dihydrogene H2");
    lcd.setCursor(0,1);
    if(c>=0) lcd.print(String(c)+" ppm");
    else lcd.print("Neant");
    int ConcentrationToInt = int(float(c*10));
    Bargraph(0,DIHYDROGENE_VALEUR_SUP*10,ConcentrationToInt);
}
/*****************************************/
/* FIN FONCTION AFFICHAGE DIHYDROGENE H2 */
/*****************************************/

/* ----------------------------------------------------------------------------------*/

/*************************************/
/* FONCTION AFFICHAGE ETHANOL C2H50H */
/*************************************/
void AffichageEthanol()
{
    float c;
    c = gas.measure_C2H5OH();
    lcd.clear();
    lcd.print("Ethanol C2H50H");
    lcd.setCursor(0,1);
    if(c>=0) lcd.print(String(c)+" ppm");
    else lcd.print("Neant");
    Bargraph(0,ETHANOL_VALEUR_SUP,c);
}
/*****************************************/
/* FIN FONCTION AFFICHAGE ETHANOL C2H50H */
/*****************************************/

/* ----------------------------------------------------------------------------------*/

/*********************/
/* FONCTION BARGRAPH */
/*********************/
void Bargraph(int limiteInf, int limiteSup, int valeur)
{
  int ValeurMap = map (valeur, limiteInf, limiteSup, 0, 10);
  if (selectionBack == mesure)
  {
    bar.setLevel(ValeurMap);
  }
  else
  {
      for (int i = 0; i <= ValeurMap; i++)
      {
        bar.setLevel(i);
        delay(50);
      }
      selectionBack = mesure;
  }
}
/*************************/
/* FIN FONCTION BARGRAPH */
/*************************/

/* ----------------------------------------------------------------------------------*/

/***********************************/
/* FONCTION BARGRAPH INVERSE FLOAT */
/***********************************/
void Bargraph2(int limiteInf, int limiteSup, int valeur)
{
  int ValeurMap = map (valeur, limiteInf, limiteSup, 10, 0);
  if (selectionBack == mesure)
  {
    bar.setLevel(ValeurMap);
  }
  else
  {
      for (int i = 0; i <= ValeurMap; i++)
      {
        bar.setLevel(i);
        delay(50);
      }
      selectionBack = mesure;
  }
}
/*************************/
/* FIN FONCTION BARGRAPH */
/*************************/

/* ----------------------------------------------------------------------------------*/

/*************************/
/* FONCTION PRECHAUFFAGE */
/*************************/
void Prechauffage()
{
  bool boutonInit = false;
   for(int i=60*PRE_HEAT_TIME; i>=0; i--)
    {
        lcd.clear();
        lcd.print("PRECHAUFFAGE");
        lcd.setCursor(0,1);
        lcd.print("ATTENTE("+String(i/60)+":"+String(i%60)+")");
        delay(1000);
        boutonInit = (digitalRead(BOUTON_SUIVANT)&&digitalRead(BOUTON_PRECEDENT));
        if (boutonInit) break;
    }
}
/*****************************/
/* FIN FONCTION PRECHAUFFAGE */
/*****************************/





