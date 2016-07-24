#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Keypad.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <OneWire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <math.h>
#include <Adafruit_ADS1015.h>

/*DS18B20 CONFIGURATION  | RTC CONFIGURATION
* merah - vcc            | SCL - 20 - merah
* abu - ground           | SDA - 21 - coklat
* kuning - data - 2     |
*/

/*SD CARD CONFIGURATION   | TEMT6000 CONFIGURATION
 * SS = 53;   - BIRU             | 1 - VCC
 * SCK = 52;  - ABU              | OUT - AD1115 A0
 * MOSI = 51; - UNGU             | J1 - GROUND
 * MISO = 50; - PUTIH
 * 3 volt vcc
 */

/* MPX CONFIGURATION
 * KAKI 1 - AD0
 * KAKI 2 - GND
 * KAKI 3 - VCC
 */

/* LCD CONFIGURATION
 * RS - 31
 * EN - 33
 * D4 - 35
 * D5 - 37
 * D6 - 39
 * D7 - 41
 */

File myFile;
LiquidCrystal lcd(31, 33, 35, 37, 39, 41);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {
    '1', '2', '3'
  }
  ,
  {
    '4', '5', '6'
  }
  ,
  {
    '7', '8', '9'
  }
  ,
  {
    '*', '0', '#'
  }
};

byte rowPins[ROWS] = {
  30, 32, 34, 36
}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  38, 40, 42
}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//global variable
char str[13];
int hari, bulan, tahun, jam, menit;
Adafruit_ADS1115 ads1115;

//DS18B20
const byte DS18B20_PIN = 2; //sensor data pin
OneWire ds(DS18B20_PIN);
byte addr[8];
float DS18B20float;

char tipe[2];
char reef[2];
char stas[5];
char ulangan[2];
char jarak[6];
char form[3];
char genus[4];
char x, z, key;
char var = 1;
unsigned int count, b, c;
String y, PIT = "", P = "";

tmElements_t tm;
const int numAvg = 20;
float mpx;
long readings = 0;

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  lcd.begin(16, 2);
  lcd.print("--- Coral ID ---");
  lcd.setCursor(0, 1);
  lcd.print("---  System  ---");
  delay(2000);
  lcd.clear();

  //SD CARD INIT
  lcd.setCursor(0, 0);
  lcd.print("Init SD card...");
  delay(1000);
  lcd.setCursor(0, 1);
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);

  if (!SD.begin(53))
  {
    lcd.print("Init failed!");
    delay(500);
    return;
  }
  lcd.print("Init finish.");

  delay(1000);
  lcd.clear();

  //DS18B20 INIT
  if ( !ds.search(addr)) {
    lcd.setCursor(0, 0);
    lcd.print("DS18B20 ERROR");
    delay(500);
    return;
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("DS18B20 OK");
    delay(500);
  }

  delay (200);
  lcd.clear();

  //REAL TIME CLOCK
  tmElements_t tm;
  if (RTC.read(tm)) {
    lcd.setCursor(0, 1);
    print2digits(tm.Hour);
    jam = tm.Hour;

    lcd.write(':');
    print2digits(tm.Minute);
    menit = tm.Minute;

    lcd.write(':');
    print2digits(tm.Second);

    lcd.setCursor(0, 0);
    lcd.print(tm.Day);
    hari = tm.Day;

    lcd.write('/');
    lcd.print(tm.Month);
    bulan = tm.Month;

    lcd.write('/');
    lcd.print(tmYearToCalendar(tm.Year));
    tahun = tmYearToCalendar(tm.Year);
  }
  else {
    if (RTC.chipPresent()) {
      lcd.print("Error.Please run the SetTime");
      return;
    } else {
      lcd.print("Check the circuitry.");
      return;
    }
    delay(1000);
  }
  delay(1000);
  lcd.clear();

  //INISIALISASI ADS115
  ads1115.begin();

  //PEMILIHAN METODE
  lcd.print("CHOOSE METHOD: ");
  lcd.setCursor(0, 1);
  lcd.print("1.LIT 2.PIT ");
  lcd.setCursor(15, 0);

}

void loop() {
  mpx = 0;

  //TIPE PENGUKURAN
  while (var == 1)
  {
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2')
      { lcd.setCursor(15, 0);
        lcd.print(key);
        if (key == '1')
        {
          tipe[0] = 'L';
        }
        if (key == '2')
        {
          tipe[0] = 'P';
        }
      }

      if (key == '#')
      {
        lcd.setCursor(0, 1);
        lcd.print("            ");
        delay(500);
        lcd.clear();
        z = 0;
        b = 0;
        break;
      }
    }
  }

  lcd.print("STATION = ");
  z = 10;
bailout:
  while (var == 1)
  {
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0')
      {
        lcd.write(key);
        stas[b] = key;
        b++;
        z++;
      }

      if (key == '#')
      {
        lcd.setCursor(0, 1);
        lcd.print("----------------");
        delay(500);
        lcd.clear();
        z = 0;
        b = 0;
        break;
      }

      if (key == '*')
      {
        stas[z - 1] = ' ';
        z--;
        lcd.setCursor(z, 0);
        lcd.print(" ");
        lcd.setCursor(z, 0);

      }
    }
  }

  lcd.print("REPETITION = ");
  z = 13;

  while (var == 1)
  {
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0')
      {
        lcd.write(key);
        ulangan[b] = key;
        b++;
        z++;
      }

      if (key == '#')
      {
        lcd.setCursor(0, 1);
        lcd.print("----------------");
        delay(500);
        lcd.clear();
        z = 0;
        b = 0;
        break;
      }

      if (key == '*')
      {
        stas[z - 1] = ' ';
        b--;
        z--;
        lcd.setCursor(z, 0);
        lcd.print(" ");
        lcd.setCursor(z, 0);

      }
    }
  }
  
  String filename = String(tipe) + "IT_" + String(stas) + "_" + String(ulangan) + ".txt";
  filename.toCharArray(str, 13);

  if (SD.exists(str))
  {
    lcd.print("    ALREADY ");
    lcd.setCursor(0, 1);
    lcd.print("   AVAILABLE");
    delay(2000);
    lcd.clear();
    lcd.print("STATION = ");
    goto bailout;
  }
  else
  {
    lcd.print("  NEW STATION");
    myFile = SD.open(str, FILE_WRITE);
    myFile.flush();
    myFile.close();
    delay(1500);
    lcd.clear();
  }

  //bersihkan variabel stnd
  for ( int i = 0; i < sizeof(stas); ++i )
    stas[i] = (char)0;

  //TIPE REEF - REEF TYPE
  lcd.print("REEF TYPE = ");
  while (var == 1)
  {
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3')
      { lcd.setCursor(15, 0);
        lcd.print(key);
        if (key == '1')
        {
          reef[0] = '1';//flat
        }
        if (key == '2')
        {
          reef[0] = '2';//slope
        }
        if (key == '3')
        {
          reef[0] = '3';//wall
        }
      }

      if (key == '#')
      { myFile = SD.open(str, FILE_WRITE);
        myFile.print("REEF TYPE");
        myFile.print("|");
        myFile.println(reef);
        myFile.close();
        delay(500);
        lcd.clear();
        break;
      }
    }
  }

  //PENGUKURAN TEMT6000
  int16_t adc0 = ads1115.readADC_SingleEnded(0);
  if (adc0 > 26000)
  {
    adc0 = 26000;
  }
  float beam = (float)adc0 / 26000; //TRANSMISSION
  float c = -1 / 0.1 * log(float(beam));  //CT
  float coef = float(c) * 0.9 + 0.081;    //ATTENUATION COEF
  float vis = 4.55 / (float)coef;

  //TAMPILAN TEMT6000
  lcd.print("DN=");
  lcd.print(adc0);
  lcd.setCursor(9, 0);
  lcd.print("CT=");
  lcd.print(c, 2);
  lcd.setCursor(0, 1);
  lcd.print("AT=");
  lcd.print(coef, 2);
  lcd.setCursor(9, 1);
  lcd.print("Vi=");
  lcd.print(vis, 2); // set angka di belakang koma 2 digit

  //PENYIMPANAN DATA TEMT6000
  myFile = SD.open(str, FILE_WRITE);
  myFile.print("DN TEMT6000");
  myFile.print("|");
  myFile.println(adc0);
  myFile.print("BEAM (%)");
  myFile.print("|");
  myFile.println(beam);
  myFile.print("CT");
  myFile.print("|");
  myFile.println(c);
  myFile.print("ATTENUATION COEFFICIENT");
  myFile.print("|");
  myFile.println(coef);
  myFile.print("VISIBILITAS");
  myFile.print("|");
  myFile.println(vis, 2); // set angka di belakang koma 2 digit
  myFile.close();

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);

  //PENGUKURAN SUHU
  DS18B20float = getTemp();

  //TAMPILAN SUHU
  lcd.print("Temp : ");
  lcd.print(DS18B20float);
  lcd.print((char)223);
  lcd.print("C");

  //PENYIMPANAN DATA SUHU
  myFile = SD.open(str, FILE_WRITE);
  myFile.print("Temperature (");
  myFile.print((char)223);
  myFile.print("C)");
  myFile.print("|");
  myFile.println(DS18B20float);
  myFile.close();
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("  Pengukuran");
  lcd.setCursor(0, 1);
  lcd.print("  Kedalaman");
  //PENGUKURAN TEKANAN MPX
  for (byte i = 0; i < numAvg; i++)
  {
    readings += analogRead(A0);
    delay(100);
  }

  mpx = (readings / numAvg) - 168;

  float volt = (float(mpx) * 4.5) / 1024.0 + 0.2;
  float kpa = (volt - 0.2) / (0.0012858 * 5 ); //KILOpascal
  float depth = kpa * 0.102; // DALAM METER

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);

  //TAMPILAN SENSOR TEKANAN MPX
  lcd.print("DN=");
  lcd.print(mpx, 0);
  lcd.setCursor(10, 0);
  lcd.print("V=");
  lcd.print(volt, 2);
  lcd.setCursor(0, 1);
  lcd.print("KPA=");
  lcd.print(kpa, 2);
  lcd.setCursor(10, 1);
  lcd.print("D=");
  lcd.print(depth, 2); // set angka di belakang koma 2 digit

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);

  //MPX
  myFile = SD.open(str, FILE_WRITE);
  myFile.print("DN MPX");
  myFile.print("|");
  myFile.println(mpx, 0);
  myFile.print("VOLT");
  myFile.print("|");
  myFile.println(volt);
  myFile.print("KPA");
  myFile.print("|");
  myFile.println(kpa);
  myFile.print("DEPTH");
  myFile.print("|");
  myFile.println(depth);
  myFile.close();

  if (tipe[0] == 'P')
  {
    inputpit();
  }
  else
  {
    inputlit();
  }
}


void inputpit()
{ mpx = 0;
  myFile = SD.open(str, FILE_WRITE);
  myFile.print("TANGGAL");
  myFile.print("|");
  myFile.print("DISTANCE");
  myFile.print("|");
  myFile.print("FORM");
  myFile.print("|");
  myFile.println("DEPTH");
  myFile.close();



  lcd.setCursor(0, 0);
  lcd.print("DISTANCE|FORM");
  lcd.setCursor(0, 1);
  b = 1;
  c = 50;
  unsigned int d = 2;
  lcd.print(c);
  lcd.print("|");
  while (var == 1)
  {
    PIT = String(c);
    PIT = PIT + '|';
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5')
      {
        lcd.setCursor(d, 1);
        lcd.print(key);
        z = key;
      }
      if (key == '#')
      {
        b++;
        P = PIT + z;

        //penyimpanan data TANGGAL
        RTC.read(tm);
        myFile = SD.open(str, FILE_WRITE);
        //TANGGAL
        save2digits(tm.Day);
        myFile.write('/');
        save2digits(tm.Month);
        myFile.write('/');
        myFile.print(tmYearToCalendar(tm.Year));
        myFile.write(' ');
        save2digits(tm.Hour);
        myFile.write(':');
        save2digits(tm.Minute);
        myFile.write(':');
        save2digits(tm.Second);
        myFile.print('|');
        myFile.print(P);
        myFile.close();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(P);

        PIT = "";
        c = b * 50;
        if (c < 100)
        {
          d = 3;
        }
        if (c >= 100 && c < 1000)
        {
          d = 4;
        }
        if (c >= 1000)
        {
          d = 5;
        }
        lcd.setCursor(0, 1);
        lcd.print(c);
        lcd.print("|");

        //PENGUKURAN TEKANAN MPX
        for ( byte i = 0; i < numAvg; i++) // maximum 60 will fit in a unsigned int.
        {
          readings += analogRead(A0);
        }

        mpx = (readings / numAvg) - 168;

        float volt = (float(mpx) * 4.5) / 1024.0 + 0.2;
        float kpa = (volt - 0.2) / (0.0012858 * 5 ); //KILOpascal
        float depth = kpa * 0.102; // DALAM METER

        //PENYIMPANAN DATA DEPTH
        myFile = SD.open(str, FILE_WRITE);
        myFile.print("|");
        myFile.println(depth);
        myFile.close();

                mpx = 0;
        readings = 0;
        volt = 0;
        kpa = 0;
        depth = 0;
      }
    }
  }
}


void inputlit()
{
  mpx = 0;
  //HEADER DATA
  myFile = SD.open(str, FILE_WRITE);
  myFile.print("TANGGAL");
  myFile.print("|");
  myFile.print("DISTANCE");
  myFile.print("|");
  myFile.print("FORM");
  myFile.print("|");
  myFile.print("GENUS");
  myFile.print("|");
  myFile.println("DEPTH");
  myFile.close();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Dist|Form|Genus");
  lcd.setCursor(0, 1);
  PIT = "0";
dist:

  //JARAK
  char z = 0;
  count = 0;
  lcd.setCursor(0, 1);
  lcd.print("DIST = ");
  lcd.setCursor(10, 1);
  lcd.print("      ");
  while (var == 1)
  {
    key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0' && count < 5)
      {
        z++;
        count++;

        if (count == 1)
        {
          jarak[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(jarak);
        }
        if (count >= 2 && count < 4)
        {
          jarak[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(jarak);
        }
        if (count == 4)
        {
          jarak[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(jarak);
        }
      }
      if (key == '*')
      {
        jarak[count - 1] = ' ';
        lcd.setCursor(10, 1);
        lcd.print("      ");
        lcd.setCursor(16 - count, 1);
        lcd.print(jarak);
        z--;
        count--;
      }
      if (key == '#')
      { P = String(jarak);
        if (P.toInt() <= PIT.toInt())
        { for ( int i = 0; i < sizeof(jarak); ++i )
            jarak[i] = (char)0;
          goto dist;

        }

        //penyimpanan data TANGGAL
        RTC.read(tm);
        myFile = SD.open(str, FILE_WRITE);
        //TANGGAL
        save2digits(tm.Day);
        myFile.write('/');
        save2digits(tm.Month);
        myFile.write('/');
        myFile.print(tmYearToCalendar(tm.Year));
        myFile.write(' ');
        save2digits(tm.Hour);
        myFile.write(':');
        save2digits(tm.Minute);
        myFile.write(':');
        save2digits(tm.Second);
        myFile.print('|');
        y = jarak;

        myFile.print(jarak);
        myFile.print('|');
        myFile.close();

        if (y.toInt() < 10)
        { lcd.setCursor(0, 0);
          lcd.print("    ");
          lcd.print(jarak[0]);

        }
        if (y.toInt() >= 10 & y.toInt() < 100)
        { lcd.setCursor(0, 0);
          lcd.print("   ");
          lcd.print(jarak[0]);
          lcd.print(jarak[1]);
        }
        if (y.toInt() >= 100 & y.toInt() < 1000)
        { lcd.setCursor(0, 0);
          lcd.print("  ");
          lcd.print(jarak[0]);
          lcd.print(jarak[1]);
          lcd.print(jarak[2]);
        }
        if (y.toInt() >= 1000)
        { lcd.setCursor(0, 0);
          lcd.print(" ");
          lcd.print(jarak);
        }

        myFile = SD.open(str, FILE_WRITE);
        z = 0;
        count = 0;
        PIT = jarak;
        for ( int i = 0; i < sizeof(jarak); ++i )
          jarak[i] = (char)0;
        break;

      }
    }
  }

  //BENTUK
  lcd.setCursor(0, 1);
  lcd.print("FORM = ");
  lcd.setCursor(10, 1);
  lcd.print("      ");
  while (var == 1)
  {
    key = keypad.getKey();

    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0' && count < 3)
      {
        z++;
        count++;
        if (count == 1)
        {
          form[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(form);

        }
        if (count == 2)
        {
          form[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(form);
          y = form;

          if (y.toInt() > 28)
          {
            lcd.setCursor  (14, 1);
            lcd.print("  ");

            z = 0;
            count = 0;
          }
        }
        if (count > 2)
        { form[1] = 0;
          count = 0;
          lcd.setCursor  (14, 1);
          lcd.print("  ");

          z = 0;
          count = 0;
        }
      }
    }
    if (key == '*')
    {
      form[count - 1] = ' ';
      lcd.setCursor(10, 1);
      lcd.print("      ");
      lcd.setCursor(16 - count, 1);
      lcd.print(form);
      z--;
      count--;
    }
    if (key == '#')
    {
      if (count == 1)
      {
        myFile.print('0');
        myFile.print(form[0]);
        myFile.print('|');
        myFile.close();
        myFile = SD.open(str, FILE_WRITE);
        lcd.setCursor(6, 0);
        lcd.print("  ");
        lcd.print('0');
        lcd.print(form[0]);
        for ( int i = 0; i < sizeof(form); ++i )
          form[i] = (char)0;
        z = 0;
        count = 0;
        break;
      }
      if (count == 2 && y.toInt() <= 13)
      {
        myFile.print(form[0]);
        myFile.print(form[1]);
        myFile.print('|');
        myFile.close();
        myFile = SD.open(str, FILE_WRITE);
        lcd.setCursor(6, 0);
        lcd.print("  ");
        lcd.print(form[0]);
        lcd.print(form[1]);
        for ( int i = 0; i < sizeof(form); ++i )
          form[i] = (char)0;
        z = 0;
        count = 0;
        break;
      }
      if (count == 2 && y.toInt() > 13)
      {
        //PENGUKURAN TEKANAN MPX
        for (byte i = 0; i < numAvg; i++)
        {
          readings += analogRead(A0);
          delay(100);
        }
        mpx = (readings / numAvg) - 168;

        float volt = (float(mpx) * 4.5) / 1024.0 + 0.2;
        float kpa = (volt - 0.2) / (0.0012858 * 5 ); //KILOpascal
        float depth = kpa * 0.102; // DALAM METER

        //PENYIMPANAN DATA DEPTH
        myFile = SD.open(str, FILE_WRITE);
        myFile.print(form[0]);
        myFile.print(form[1]);
        myFile.print('|');
        myFile.print('1');
        myFile.print("|");
        myFile.println(depth);
        myFile.close();
        lcd.setCursor(6, 0);
        lcd.print("  ");
        lcd.print(form[0]);
        lcd.print(form[1]);
        lcd.setCursor(11, 0);
        lcd.print("    ");
        lcd.print('1');
        for ( int i = 0; i < sizeof(form); ++i )
          form[i] = (char)0;
        z = 0;
        count = 0;
                mpx = 0;
        readings = 0;
        volt = 0;
        kpa = 0;
        depth = 0;
        goto dist;
      }
    }
  }


  //GENERA
  lcd.setCursor(0, 1);
  lcd.print("GENUS = ");
  lcd.setCursor(10, 1);
  lcd.print("      ");
  while (var == 1)
  {
    key = keypad.getKey();

    if (key != NO_KEY)
    {
      if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0' && count < 3)
      {
        z++;
        count++;
        if (count == 1)
        {
          genus[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(genus);
        }
        if (count == 2)
        {
          genus[count - 1] = key;
          lcd.setCursor(16 - count, 1);
          lcd.print(genus);
        }
        if (count == 3)
        {
          genus[count - 1] = key;
          y = String(genus[0]) + String(genus[1]) + String(genus[2]);

          if (y.toInt() > 111)
          {
            genus[count - 1] = ' ';
            lcd.setCursor(10, 1);
            lcd.print("      ");
            lcd.setCursor(16 - count, 1);
            lcd.print(genus);
            z--;
            count--;
          }
          lcd.setCursor(13, 1);
          lcd.print(genus);
        }
      }
      if (key == '*')
      {
        genus[count - 1] = ' ';
        lcd.setCursor(10, 1);
        lcd.print("      ");
        lcd.setCursor(16 - count, 1);
        lcd.print(genus);
        z--;
        count--;
      }
      if (key == '#')
      {
        if (count == 1)
        {
          myFile.print('0');
          myFile.print(genus[0]);
          myFile.close();
          myFile = SD.open(str, FILE_WRITE);
          myFile.close();
          lcd.setCursor(11, 0);
          lcd.print("    ");
          lcd.print(genus[0]);
          for ( int i = 0; i < sizeof(genus); ++i )
            genus[i] = (char)0;
          z = 0;
          count = 0;
        }
        if (count == 2)
        {
          myFile.print(genus[0]);
          myFile.print(genus[1]);
          myFile.close();
          myFile = SD.open(str, FILE_WRITE);
          myFile.close();
          lcd.setCursor(11, 0);
          lcd.print("   ");
          lcd.print(genus[0]);
          lcd.print(genus[1]);
          for ( int i = 0; i < sizeof(genus); ++i )
            genus[i] = (char)0;
          z = 0;
          count = 0;
        }
        if (count == 3)
        {
          myFile.print(genus[0]);
          myFile.print(genus[1]);
          myFile.print(genus[2]);
          myFile.close();
          myFile = SD.open(str, FILE_WRITE);
          myFile.close();
          lcd.setCursor(11, 0);
          lcd.print("  ");
          lcd.print(genus[0]);
          lcd.print(genus[1]);
          lcd.print(genus[2]);
          for ( int i = 0; i < sizeof(genus); ++i )
            genus[i] = (char)0;
          z = 0;
          count = 0;
        }

        //PENGUKURAN TEKANAN MPX
        for (byte i = 0; i < numAvg; i++)
        {
          readings += analogRead(A0);
          delay(100);
        }
        mpx = (readings / numAvg) - 168;

        float volt = (float(mpx) * 4.5) / 1024.0 + 0.2;
        float kpa = (volt - 0.2) / (0.0012858 * 5 ); //KILOpascal
        float depth = kpa * 0.102; // DALAM METER

        //PENYIMPANAN DATA DEPTH
        myFile = SD.open(str, FILE_WRITE);
        myFile.print("|");
        myFile.println(depth);
        myFile.close();
                mpx = 0;
        readings = 0;
        volt = 0;
        kpa = 0;
        depth = 0;

        goto dist;
      }
    }
  }
}
//harus masukkan satu data tambahan untuk menutup file agar data terakhir tidak hilang


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}

void save2digits(int number) {
  if (number >= 0 && number < 10) {
    myFile.print('0');
  }
  myFile.print(number);
}

// watchdog interrupt
ISR (WDT_vect)
{
  wdt_disable(); // disable watchdog
} // end of WDT_vect

// this returns the temperature from one DS18S20 in DEG Celsius using 12 bit conversion
float getTemp() {
  byte data[2];
  ds.reset();
  ds.select(addr);
  ds.write(0x44); // start conversion, read temperature and store it in the scratchpad

  //this next bit creates a 1 second WDT delay during the DS18b20 temp conversion
  //The time needed between the CONVERT_T command and the READ_SCRATCHPAD command has to be at least
  //750 millisecs (but can be shorter if using a D18B20 type with resolutions < 12 bits)
  MCUSR = 0; // clear various “reset” flags
  WDTCSR = bit (WDCE) | bit (WDE); // allow changes, disable reset
  // set interrupt mode and an interval
  WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1); //a 1 sec timer
  wdt_reset(); // pat the dog
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu ();
  sleep_disable(); // cancel sleep after wakeup as a precaution

  byte present = ds.reset();  //now we can read the temp sensor data
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  for (int i = 0; i < 2; i++) { // Only read the bytes you need? there is more there
    data[i] = ds.read();
  }
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB); //using two’s compliment
  float TemperatureSum = tempRead / 16; //this converts to C
  return TemperatureSum;

}
