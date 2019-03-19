/*   STM32F103C8 Blue Pill ( PC13)

  Serialport set and display RTC clock , Write by CSNOL https://github.com/csnol/STM32-Examples
  based on https://github.com/rogerclarkmelbourne/Arduino_STM32
*/


#include <RTClock.h>

#define PTT_TIMOUT 3
#define MORSETAB 39
#define ALRMRLD 900 //15min*60sec

#define segG PB3 
#define segF PB4 
#define segA PB5 
#define segB PB6 
#define segE PB7 
#define segDash PB8 
#define segC PB9 
#define segDit PA10
#define pinPTT PA15
#define pinSQL PA9
#define pinSND PB15
#define pinGRN PA8

RTClock rtclock (RTCSEL_LSE,0x7fff); // 0x7fff
int timezone = 2;      // change to your timezone
time_t tt, tt1;
tm_t mtt;
uint8_t dateread[11];
bool dispflag = true;

uint8_t ts=PTT_TIMOUT+2;
bool alarmflag = false;
bool bipflag = false;
bool callorQTH = true;
uint8_t countSQL=0;
//-----------------------------------------------------------------------------
                           //B3456789
const char  sevenseg[] =  {0b00111111,/*0*/
                           0b00001001,/*1*/
                           0b01011110,/*2*/
                           0b01011011,/*3*/
                           0b01101001,/*4*/
                           0b01110011,/*5*/
                           0b01110111,/*6*/
                           0b00011001,/*7*/
                           0b01111111,/*8*/
                           0b01111011};/*9*/

//-----------------------------------------------------------------------------
int morse[2][MORSETAB] = {
    {'A','B','C','D','E','F',
     'G','H','I','J','K','L',
     'M','N','O','P','Q','R',
     'S','T','U','V','W','X',
     'Y','Z','0','1','2','3',
     '4','5','6','7','8','9',
     ' '},
    {B00000110,B00010111,B00010101,B00001011,B00000011,B00011101,
     B00001001,B00011111,B00000111,B00011000,B00001010,B00011011,
     B00000100,B00000101,B00001000,B00011001,B00010010,B00001101,
     B00001111,B00000010,B00001110,B00011110,B00001100,B00010110,
     B00010100,B00010011,B00100000,B00110000,B00111000,B00111100,
     B00111110,B00111111,B00101111,B00100111,B00100011,B00100001,
     B00101110}
};

int wpm=12;
int frq=1000;
int del=3000;
int bip=500;
int ditpause=1200/wpm;
int dashpause=ditpause*3;
int elementpause=ditpause;
int charpause=ditpause*3;
int wordpause=ditpause*7;

//-----------------------------------------------------------------------------
const char * weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char * months[] = {"Dummy", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
//-----------------------------------------------------------------------------

char segout(char num)
{
  digitalWrite(segG, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segDash, HIGH);
  digitalWrite(segC, HIGH);

  if(num==0xFF)
  {
    return 0;
  }
  
  if (sevenseg[num] & 0b01000000)
  {
    digitalWrite(segG, LOW);
  }

  if (sevenseg[num] & 0b00100000)
  {
    digitalWrite(segF, LOW);
  }

  if (sevenseg[num] & 0b00010000)
  {
    digitalWrite(segA, LOW);
  }

  if (sevenseg[num] & 0b00001000)
  {
    digitalWrite(segB, LOW);
  }

  if (sevenseg[num] & 0b00000100)
  {
    digitalWrite(segE, LOW);
  }
  
  if (sevenseg[num] & 0b00000010)
  {
    digitalWrite(segDash, LOW);
  }
  
  if (sevenseg[num] & 0b00000001)
  {
    digitalWrite(segC, LOW);
  }
  return 1;
}

uint8_t str2month(const char * d)
{
    uint8_t i = 13;
    while ( (--i) && strcmp(months[i], d)!=0 );
    return i;
}
//-----------------------------------------------------------------------------
const char * delim = " :";
char s[128]; // for sprintf
//-----------------------------------------------------------------------------
void ParseBuildTimestamp(tm_t & mt)
{
    // Timestamp format: "Dec  8 2017, 22:57:54"
    sprintf(s, "Timestamp: %s, %s\n", __DATE__, __TIME__);
    //Serial.print(s);
    char * token = strtok(s, delim); // get first token
    // walk through tokens
    while( token != NULL ) {
        uint8_t m = str2month((const char*)token);
        if ( m>0 ) {
            mt.month = m;
            //Serial.print(" month: "); Serial.println(mt.month);
            token = strtok(NULL, delim); // get next token
            mt.day = atoi(token);
            //Serial.print(" day: "); Serial.println(mt.day);
            token = strtok(NULL, delim); // get next token
            mt.year = atoi(token) - 1970;
            //Serial.print(" year: "); Serial.println(mt.year);
            token = strtok(NULL, delim); // get next token
            mt.hour = atoi(token);
            //Serial.print(" hour: "); Serial.println(mt.hour);
            token = strtok(NULL, delim); // get next token
            mt.minute = atoi(token);
            //Serial.print(" minute: "); Serial.println(mt.minute);
            token = strtok(NULL, delim); // get next token
            mt.second = atoi(token);
            //Serial.print(" second: "); Serial.println(mt.second);
        }
        token = strtok(NULL, delim);
    }
}

void alarmset(void)
{
  rtclock.detachAlarmInterrupt();
  rtclock.createAlarm(blink, (rtclock.getTime() + ALRMRLD)); 
}

//-----------------------------------------------------------------------------
// This function is called in the attachSecondsInterrupt
//-----------------------------------------------------------------------------
void SecondCount ()
{
  tt++;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  if ((ts>=0)&&(ts<=PTT_TIMOUT))
  {
    segout(ts);
    if (ts==0)
    {
     bipflag = true;
    }
    ts--;
    digitalWrite(pinPTT, HIGH);
  }
  else
  {
    segout(0xFF);
    //digitalWrite(pinPTT, LOW);
  }
}
//-----------------------------------------------------------------------------
// This function is called in the attachAlarmInterrupt
//-----------------------------------------------------------------------------
void blink ()
{
  //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  alarmflag = true;
  //Serial.println(" Blink ");
}

void setup()
{

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(pinPTT, OUTPUT);
  pinMode(pinGRN, OUTPUT);
  pinMode(segDit, OUTPUT);
  pinMode(segG, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segDash, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(pinSND, OUTPUT);
  pinMode(pinSQL,INPUT);
  
  digitalWrite(segG, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segDash, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segDit, HIGH);
  digitalWrite(pinPTT, LOW);
  digitalWrite(pinSND, LOW);
  //digitalWrite(LED_BUILTIN, HIGH);
  
  //while (!Serial); delay(1000);
  
  //ParseBuildTimestamp(mtt);  // get the Unix epoch Time counted from 00:00:00 1 Jan 1970
  //tt = rtclock.makeTime(mtt) + 25; // additional seconds to compensate build and upload delay
  //rtclock.setTime(tt);
  tt1 = tt;
  rtclock.attachAlarmInterrupt(blink);// Call blink
  rtclock.attachSecondsInterrupt(SecondCount);// Call SecondCount

  alarmset();   
}


char playmorse(char* morsearray)
{

  char arindx=0;
  while (arindx!=strlen(morsearray))
  {
       char i=0;
       while (i<MORSETAB)
        {
          
          if (*(morsearray+arindx)==' ')
          {
            delay(wordpause);
            arindx++;
            i=0;
          }
          else if (*(morsearray+arindx)==morse[0][i])
          {

            unsigned char charindx=B10000000;
            while ((morse[1][i]&charindx)==0)
            {
              charindx=charindx>>1;
            }
            charindx=charindx>>1;
            while (charindx!=0)
            {
              //Serial.write(morse[1][i]&&charindx);
              //Serial.println(charindx,DEC);
              
              if ((morse[1][i]&charindx)>0)
              {
                 //Serial.print(" DIT");
                 //digitalWrite(LED_BUILTIN, LOW);
                 digitalWrite(segDit, LOW);
                 tone(pinSND, frq);   
                 delay(ditpause);                  
                 //digitalWrite(LED_BUILTIN, HIGH);
                 digitalWrite(segDit, HIGH);
                 noTone(pinSND);
                 delay(elementpause);
              }
              else
              {
                 //Serial.print(" DASH");
                 //digitalWrite(LED_BUILTIN, LOW);
                 digitalWrite(segDash, LOW);
                 tone(pinSND, frq);   
                 delay(dashpause);                  
                 //digitalWrite(LED_BUILTIN, HIGH);
                 digitalWrite(segDash, HIGH);
                 noTone(pinSND);
                 delay(elementpause);
              }
              
              charindx=charindx>>1;
            }
          }
          i++;
        }
    arindx++;
    delay(charpause-elementpause);
  }
  delay(del);  
}

//-----------------------------------------------------------------------------
void loop()
{
  if ( Serial.available()>10 ) {
    for (uint8_t i = 0; i<11; i++) {
	    dateread[i] = Serial.read();
    }
    Serial.flush();
    tt = atol((char*)dateread);
    rtclock.setTime(rtclock.TimeZone(tt, timezone)); //adjust to your local date

    alarmset();  
  }

  if(digitalRead(pinSQL)==LOW)
  {
    countSQL++;
    delay(50);
    if (countSQL>=3)
    {
      ts=PTT_TIMOUT+1;
      segout(0xFF);
      countSQL=0;
      alarmset();
      digitalWrite(pinGRN, HIGH);  
    }
  }
  else
  {
    if (ts==PTT_TIMOUT+1)
    {
      ts=PTT_TIMOUT;
    }
    countSQL=0;
    digitalWrite(pinGRN, LOW); 
  }
  
  if (bipflag == true)
  {
     bipflag = false;
     //digitalWrite(pinPTT, HIGH);
     delay(2*bip);
     tone(pinSND, frq);   
     delay(bip);                  
     noTone(pinSND);
     digitalWrite(pinPTT, LOW);
  }

  if(alarmflag == true)
  {
    alarmflag = false;
    alarmset();  
    Serial.println(" Alarm ");
    digitalWrite(pinPTT, HIGH);
    if (callorQTH==true)
    {
      playmorse("UR5TLZ");
      Serial.println("CallSign");
    }
    else
    {
      playmorse("KN39MJ");
      Serial.println("QTH");
    }
    callorQTH=!callorQTH;
    digitalWrite(pinPTT, LOW);
  }
  
  if (tt1 != tt && dispflag == true )
  {
    tt1 = tt;
    // get and print actual RTC timestamp
    rtclock.breakTime(rtclock.now(), mtt);
    sprintf(s, "RTC timestamp: %s %u %u, %s, %02u:%02u:%02u\n",
      months[mtt.month], mtt.day, mtt.year+1970, weekdays[mtt.weekday], mtt.hour, mtt.minute, mtt.second);
    Serial.print(s);
    if (mtt.second==0)
    {
      segout(mtt.hour/10);
      Serial.print(mtt.hour/10);
      delay(500);
      segout(mtt.hour-((mtt.hour/10)*10));
      Serial.print(mtt.hour-((mtt.hour/10)*10));
      delay(500);
      Serial.print(":");
      
      digitalWrite(segDit, LOW);
      delay(100);
      digitalWrite(segDit, HIGH);
      delay(100);
      digitalWrite(segDit, LOW);
      delay(100);
      digitalWrite(segDit, HIGH);
      delay(100);
      
      segout(mtt.minute/10);
      Serial.print(mtt.minute/10);
      delay(500);
      segout(mtt.minute-((mtt.minute/10)*10));
      Serial.println(mtt.minute-((mtt.minute/10)*10));
      delay(700);
    }
  }
}
