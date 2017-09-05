-------------------------------------------------------
1. Présentation du projet de Station de mesure de l'air
-------------------------------------------------------
Par soucis d’isolation, nos maisons sont de plus en plus hermétiques et l’air, à
l’intérieur de nos maisons, malgré le recours aux VMC, est souvent très pollué.
Cette pollution, n’est pas forcément visible, ses sources sont cependant
nombreuses et variées : matériaux utilisés dans la construction, meubles à base
de colle, produits détergents, émissions des composants électroniques… 
Le constat est alarmant et d’après plusieurs études, le niveau de pollution 
semble être nettement plus important qu’à l’extérieur de nos maisons. 

Une station, en mesure de prévenir les occupants d’un lieu d’habitation sur les
niveaux de polluant constatés dans l’air, semble être tout indiquée.
Ainsi alertés, les occupants d’un lieu seront en mesure de prendre toutes les
mesures nécessaires pour mieux vivre.
 
Cette station analysera en continu la qualité de l’air d’une pièce et donnera
une indication précise sur le type et le taux de polluants relevés dans l’air : 
	- niveau de CO2, 
	- monoxyde de carbone (CO), 
	- composants volatiles organiques (VOC), 
	- particules fines (PM).


----------------------------------
2. Description technique du projet
----------------------------------

Ce projet met en œuvre des microcontrôleurs Arduino. Ces modules se présentent
comme des cartes programmables qui permettent de concevoir de nombreux projets 
tels que : pilotage de robots, domotique… 

Ses caractéristiques techniques et son prix (une vingtaine d’euros) a favorisé 
sa diffusion auprès des passionnés d’électronique et des FabLabs en général.

-------------------------
3. Architecture du projet
-------------------------
                          |---------------|             |-------------|
                          | Affichage des |             |  Horodatage | 
                          | informations  |             |-------------|
                          |---------------|                     |
                                   ^                            |
                                   |                            V                
    |------------|          |------------|         |------------------------|
    | Capteurs   |--------->| Traitement |-------->| Sauvegarde des données |
    |------------|          |    (µc)    |         |       collectées       |
                            |------------|         |------------------------|
                                   |
                                   v
                            |------------|
                            | Saisie des |
                            | paramètres |
                            |------------|
	
----------------------------------------
4. Présentation des composants matériels
----------------------------------------
4.1 Le microcontrôleur
----------------------
Le microcontrôleur est un circuit intégré (ou IC Integrated Circuit) dont les 
tâches principales sont :
	• Le traitement des informations,
	• Le pilotage des périphériques connectés sur ses broches.

Notre choix s’est porté sur la version Arduino MEGA, qui dispose de la 
quantité de mémoire et d'un nombre d'entrées/sorties suffisants pour faire 
fonctionner l'application. 

https://www.arduino.cc/en/Main/ArduinoBoardMega2560

---------------------------------------------
4.2 Capteur de C02 infrarouge au format Grove
---------------------------------------------
Ce module Grove est basé sur le capteur infrarouge MH-Z16B et mesure le niveau
de CO2 avec une grande sensibilité. Les données sont transmises par une liaison
série

http://wiki.seeed.cc/Grove-CO2_Sensor/

------------------------------
4.3 Capteur 02 au format Grove
------------------------------
Ce module Grove permet de mesurer la concentration d'oxygène dans l'air. 
Les données transmises au microcontroleur sont de type analogique. 

http://wiki.seeed.cc/Grove-Gas_Sensor-O2/

---------------------------------------
4.4 Capteur de gaz HCHO au format Grove
---------------------------------------
Ce module basé sur le capteur de gaz WSP2110 est capable de détecter les gaz de 
type COV (composé organique volatil) comme le HCHO, le toluène, le benzène et 
l'alcool.

http://wiki.seeed.cc/Grove-HCHO_Sensor/

----------------------------------
4.5 Capteur de gaz au format Grove
----------------------------------
Ce module est équipé de 3 capteurs indépendants. Il fonctionne sur la base d'un
capteur MiCS-6814 et embarque un ATmega168PA. Les communications s'effectuent 
via une liaison I2C.

http://wiki.seeed.cc/Grove-Multichannel_Gas_Sensor/

----------------------------------------
4.6 Capteur d'humidité et de température
----------------------------------------
Ce capteur de température et d'humidité utilise une thermistance CTN et un 
capteur capacitif et délivre une sortie digitale.

http://wiki.seeed.cc/Grove-TemperatureAndHumidity_Sensor/

---------------------
4.7 Ecran LCD RVB I²C
---------------------
Ecran de 2 lignes de 16 caractères, qui affiche les informations de paramétrage 
et les menus. Un rétroéclairage peut être activé.

http://wiki.seeed.cc/Grove-LCD_RGB_Backlight/

--------------
4.8 Module RTC
--------------
L’horloge RTC (Real Time Clock) indique l'heure à la nanoseconde près. La carte 
proposée au format Grove par la société Seeed ,dont le composant central est un 
circuit DS1307.

http://wiki.seeed.cc/Grove-RTC/

--------------------------------------
4.9 Lecteur de carte µSD au format SPI
--------------------------------------
Support de stockage utilisé pour sauvegarder toutes les données au fil des 
acquisitions.

https://www.adafruit.com/product/254

---------------------------
4.10 Module bargraphe Grove
---------------------------
Basé sur un controleur à leds MY9221, ce module bargraphe est composé de 10 
segments à leds (1 rouge, 1 jaune et 8 vertes) et est utilisé pour indiquer les
nivaux de dangerosité des polluants détectés

http://wiki.seeed.cc/Grove-LED_Bar/


-------------
5. Connexions
-------------

|------------------------------------------------------------------------------|
|       Périphérique      |   Connexion et alimentation    | Protocole utilisé |
|------------------------------------------------------------------------------|
| Bouton suivant          | Entrée 6                       | Numérique         |
|------------------------------------------------------------------------------|
| Bouton précédent        | Entrée 7                       | Numérique         |
|------------------------------------------------------------------------------|
| Bouton record           | Entrée 8                       | Numérique         |
|------------------------------------------------------------------------------|
| Led Record              | Sortie 9                       | Numérique         |
|------------------------------------------------------------------------------|
| Capteur O2              | Entrée A2                      | Analogique        |
|------------------------------------------------------------------------------|
| Capteur HCHO            | Entrée A1                      | Analogique        |
|------------------------------------------------------------------------------|
| Capteur température     | Entrée 2                       | Numérique         |
|------------------------------------------------------------------------------|
| Bargraphe               | CLOCK 5                        | Numérique         |
|                         | DATA 4                         |                   |
|------------------------------------------------------------------------------|
| Moteur                  | Sortie 13                      | Numérique         |
|------------------------------------------------------------------------------|
| Multichannel_Gas_Sensor | VDD -> 5 Volts et GND (masse)  | I²C               |
|                         | SCL -> port 21 (SCL)           | I²C               |
|                         | SDA -> port 20 (SDA)           | (Adr : 0x04)      |
|                         | ADR -> GND                     |                   |
|------------------------------------------------------------------------------|
| Capteur CO2             | Vin (5Volts) et GND (masse)    | Série             |
|                         | TX -> RX1 Arduino              |                   |
|                         | RX -> TX1 Arduino              |                   |
|------------------------------------------------------------------------------|
| Lecteur de carte microSD| + -> 5 Volts et GND (masse)    | SPI               |
| (Adafruit)              | CLK -> port 52 (SCK)           |                   |
|                         | DO -> port 50 (MISO)           |                   |
|                         | DI -> port 51 (MOSI)           |                   |
|                         | CS -> port 10                  |                   |
|------------------------------------------------------------------------------|
| Module RTC (Grove) 	  | VDD -> 5 Volts et GND (masse)  | I²C               |
|                         | SCL -> port 21 (SCL)           |                   |
|                         | SDA -> port 20 (SDA)           |                   |
|------------------------------------------------------------------------------|
| Ecran LCD (Grove) 	  | VDD -> 5 Volts et GND (masse)  | I²C               |
|                         | SCL -> port 21 (SCL)           |                   |
|                         | SDA -> port 20 (SDA)           |                   |
|------------------------------------------------------------------------------|







