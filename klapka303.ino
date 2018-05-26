#include <pcf8574_esp.h>
#include <Wire.h>
#include <Servo.h>
//
//Wykorzystane: C:\Users\damian\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\libraries\Servo
//Niewykorzystane: D:\Programy\Arduino\libraries\Servo
// lista zmian
// zmiana stanu serwa/diody jedynie jak jest inny niż aktualny a nie zawsze
// dzialajca prawidlowa logika
// serwo
// czujnik swiatla
//
// polczenie zasilania:
// wtyk 5 pin 12mm --- wemos --- wtyk 8 pin 16mm
//     +5v 3A   4  --- 5V    --- 3,4 czarny z bialym paskiem
//     GND 3A   1  --- G     --- 1,2 czrny
//
//Podłaczenie urządzeń do wemosa
// wemos      ---   wtyk 8 pin 16 mm
//        D5  ---  5 zolty  czujnik swiatla
//        D6  ---  6 pomaranczowy czujnik swiatla
//        D7  ---  7 zielony  servo
//        D8  ---  8 brozowy  servo
//  Dioda LED
//  GND bezposrednio -- czarny od strony bez napisow 
//  D2 PWM pin przez rezysotr 300 Ohm od strony napisów czerowny(pomaranczowy)

//  To samo GND do przyciskow I i II
//  D4 - I - zielonyh
//  D3 - II - bialy

//serwo
//
// czujnik swiatla
//


// 8 pin 16mm --- wemos --- skrentka       --- czujniki
//         1,2 --- GND   --- brazowy        --- GND
//         3,4 --- +5V   --- niebieski      --- vcc
//           5 ---  D5   --- pomaranczowy A --- czujnik swiatla DO wyjscie cyfrowe
//           6 ---  D6   --- pomaranczowy B ---
//           7 ---  D7   --- zielony A      --- servo bialy data
//           8 ---  D8   --- zielony B      ---
//
//   A - strona prawa przyciskami do gory
//   B - strona lewa od zzlaczy
//
//  5 pin 12 mm na przyslonach
//          wtyk -------------- gniazdo --------------------- czujnik
//    1,2 GND brozowy ------ 1 GND  czarny  --------- GND serwo czarny,
//                    ------ 2 GND  czarny  --------- GND czujnik swiatla
//    3   +5V niebieski ---- 3 +5V niebieski --------- vcc serwo czerwony , vcc swiatlo
//    4   pomaranczowy  ---- 4  bialy        ---------- czujnik swiatla DO
//    5   zielony       ---- 5 zielony       ---------- serwo sygnal  bialy

// do zrobienia
// sparwadznie zmiany stanu  funkcji a nie wielokrotnie to samo
//
//
// element B to długi kabel
//
//!!!    PCF8591
//gdyby wpadalo w stan w ktorym swiatlo jest na styku ustawienia to można podlaczyc czujnik swiatla do zlacza analogowego i zrobic cos w stylu histerazy
// otwarcie nastepuje jak ilosc siwatla jest wieksza od 160 ale zamkniecie ggy ilosc siwatla spada ponizej 140
// !!!

#define ServoPinA D7 // pin z podłaczonym serwem A
#define SwiatloDA D5 // pin czyjnika swiatla A     detekcja swiatla LOW 0,  brak detekcji HIGH 1
#define ServoPinB D8 // pin z podłaczonym serwem B
#define SwiatloDB D6 // pin czyjnika swiatla B

#define LEDprzycisk D2
#define przyciskA D3 //wcisniety LOW 0, nie wcisiniety HIGH 1
#define przyciskB D4 //wcisniety LOW 0, nie wcisiniety HIGH 1

#define CzasMartwy 100 // czas martwy w milisekundach - zabezpieczenie przed drzeniem
#define Otwarte 40 // kat w stopniach otwartego serwa
#define Zamkniete 12 // kat w stopniach zamknietego serwa
#define Zapalone LOW//
#define Zgaszone HIGH//

Servo servoA;
Servo servoB;

int StanSerwo;

void setup() {

  Serial.begin(9600);
 
  pinMode(LEDprzycisk,OUTPUT);
  digitalWrite(LEDprzycisk,Zapalone);
 
  pinMode(przyciskA,INPUT_PULLUP);
  pinMode(przyciskB,INPUT_PULLUP);

  pinMode(ServoPinA, OUTPUT);
  servoA.attach(ServoPinA);
  pinMode(ServoPinB, OUTPUT);
  servoB.attach(ServoPinB);

  Serial.println("servo");

  pinMode(SwiatloDA, INPUT);
  pinMode(SwiatloDB, INPUT);

  
  StanSerwo = Zamkniete;
  servoA.write(StanSerwo); //przejdz serwem A na zamkniete
  servoB.write(StanSerwo); //przejdz serwem B na zamkniete
  digitalWrite(ServoPinA, LOW);
  digitalWrite(ServoPinB, LOW);
}

int ostatniStan=0;
unsigned long blink_czas = millis(); 

void migaj(int co,int jasnoscLOW, int jasnoscHIGH, int czasLOW, int czasHIGH ) //co - numer pinu, jasnosc[LOW,HIGH] - jasnosci zapalone zggaszne do analogWrite od 0 do 255, czas[LOW,HIGH] w milisekundach
  {
   int czekaj_do=((ostatniStan==0)?czasLOW:czasHIGH);
   int zmien_na=((ostatniStan==0)?jasnoscHIGH:jasnoscLOW);
    
   if(millis() - blink_czas > czekaj_do)
    {
      analogWrite(co,zmien_na);
     ostatniStan=((ostatniStan==0)?1:0);
     blink_czas=millis();
    }
  }


unsigned long czas = millis();

void loop() {


  
    
   if(digitalRead(przyciskA)&&digitalRead(przyciskB)) //dziala tryb auto
    {
      analogWrite(LEDprzycisk,160); // jesli auto to swiecimy stale z jasnoscia polowy 
      if(millis() - czas > CzasMartwy){
      if ((!digitalRead(SwiatloDA)|| !digitalRead(SwiatloDB))) // jezeli swiatlo
      {
        czas = millis();
        if (StanSerwo != Otwarte)
        {
          StanSerwo = Otwarte;
          servoA.write(StanSerwo); //przejdz serwem A na otwarte
          servoB.write(StanSerwo); //przejdz serwem B na otwarte
          
          //zeby uniknac dfrzenia tu mozna deatacznac serwo czy to cos daje to ja nie wiem:
          digitalWrite(ServoPinA, LOW);
          digitalWrite(ServoPinB, LOW);
        }
        
      }
      else // auto i nie ma swiatla
       { 
         if (StanSerwo != Zamkniete)
        {
          StanSerwo = Zamkniete;
          servoA.write(StanSerwo); //przejdz serwem A na zamkniete
          servoB.write(StanSerwo); //przejdz serwem B na zamkniete
        
          digitalWrite(ServoPinA, LOW);
          digitalWrite(ServoPinB, LOW);
        }
       }
       }
      }
      else //tryb manualny
      {
        if(!digitalRead(przyciskA)) // wciesnety I na przycisku zawsze otwarte
          {
           migaj(LEDprzycisk,80,255,200,800);
           //analogWrite(LEDprzycisk,80); //na razie swieci slabo stale i tyle ale docelowo migamy powoli od 80 do 200
           if (StanSerwo != Otwarte) //otweiramy jesli nie jest otwarte
           {
             StanSerwo = Otwarte;
             servoA.write(StanSerwo); //przejdz serwem A na otwarte
             servoB.write(StanSerwo); //przejdz serwem B na otwarte
          
             //zeby uniknac dfrzenia tu mozna deatacznac serwo czy to cos daje to ja nie wiem:
             digitalWrite(ServoPinA, LOW);
             digitalWrite(ServoPinB, LOW);
            }
          }

        if(!digitalRead(przyciskB)) // wciesniete II na przycisku zawsze zamkniete
        {
          migaj(LEDprzycisk,80,255,400,100);
          //analogWrite(LEDprzycisk,160);    // na razie swiecimy stale jasniej ale do celowo pulsujemy szybko od 80 do 200
          if (StanSerwo != Zamkniete) //jesli otwarte to zamykamy
           {
            StanSerwo = Zamkniete;
            servoA.write(StanSerwo); //przejdz serwem A na zamkniete
            servoB.write(StanSerwo); //przejdz serwem B na zamkniete
        
             digitalWrite(ServoPinA, LOW);
             digitalWrite(ServoPinB, LOW);
            }   
        }
     }
  }
