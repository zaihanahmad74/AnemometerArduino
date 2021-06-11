#define PIN_ANEMOMETER 2                //(1)

volatile unsigned long letzterTrigger;  //(2)

byte bitPos = -1;     //(3)
byte paketNum = -1;   //(4)
int paket1Bits[36];   //(5)
int paket2Bits[36];   //(6)

bool checksummePruefen(int bits[]) {
  int checksumme = 0xf;         //(1)
  for (int i = 0; i < 8; i++) { //(2)
    checksumme -= bits[i * 4] | bits[i * 4 + 1] << 1 | bits[i * 4 + 2] << 2 | bits[i * 4 + 3] << 3;
  }
  checksumme &= 0xf;            //(3)
  int erwarteteChecksumme = bits[32] | bits[33] << 1 | bits[34] << 2 | bits[35] << 3;   //(4)
  return checksumme == erwarteteChecksumme;     //(5)
}

void paketDekodieren() {
  if (!checksummePruefen(paket1Bits)) { //(1)
    Serial.println("Checksumme Diskrepanz im Paket #1");
    return;
  }


  if (paket1Bits[9] == 1 && paket1Bits[10] == 1) {  //(2)

    if (!checksummePruefen(paket2Bits)) {           //(3)
      Serial.println("Checksumme Diskrepanz im Paket #2");
      return;
    }

    float windGeschw = (paket1Bits[24]    | paket1Bits[25] << 1 | paket1Bits[26] << 2 | paket1Bits[27] << 3 |
                        paket1Bits[28] << 4 | paket1Bits[29] << 5 | paket1Bits[30] << 6 | paket1Bits[31] << 7) * 0.2f; //(4)
    Serial.print("Durchschnittliche Windgeschwindigkeit: ");
    Serial.print(windGeschw);
    Serial.println(" m/s"); //(5)


    float windStoss = (paket2Bits[24]    | paket2Bits[25] << 1 | paket2Bits[26] << 2 | paket2Bits[27] << 3 |
                       paket2Bits[28] << 4 | paket2Bits[29] << 5 | paket2Bits[30] << 6 | paket2Bits[31] << 7) * 0.2f; //(6)
    Serial.print("Windstoss: ");
    Serial.print(windStoss);
    Serial.println(" m/s"); //(7)

    int windRichtung = (paket2Bits[15]    | paket2Bits[16] << 1 | paket2Bits[17] << 2 | paket2Bits[18] << 3 |
                        paket2Bits[19] << 4 | paket2Bits[20] << 5 | paket2Bits[21] << 6 | paket2Bits[22] << 7 |
                        paket2Bits[23] << 8);  //(8)
    Serial.print("Windrichtung: ");
    Serial.print(windRichtung);
    Serial.println(" °\n");//(9)

  } /*else {
    int temperaturRoh = (paket1Bits[12]    | paket1Bits[13] << 1 | paket1Bits[14] << 2 | paket1Bits[15] << 3 |
                          paket1Bits[16] << 4 | paket1Bits[17] << 5 | paket1Bits[18] << 6 | paket1Bits[19] << 7 |
                          paket1Bits[20] << 8 | paket1Bits[21] << 9 | paket1Bits[22] << 10 | paket1Bits[23] << 11);
    if (temperaturRoh & 0x800) temperaturRoh += 0xF000;
    float temperatur = temperaturRoh * 0.1f;
    Serial.print("Temperatur: ");
    Serial.print(temperatur);
    Serial.println(" °C");

    int luftfeuchtigkeit = (paket1Bits[24] | paket1Bits[25] << 1 | paket1Bits[26] << 2 | paket1Bits[27] << 3 ) +
                   (paket1Bits[28] | paket1Bits[29] << 1 | paket1Bits[30] << 2 | paket1Bits[31] << 3 ) * 10;
    Serial.print("Luftfeuchtigkeit: ");
    Serial.print(luftfeuchtigkeit);
    Serial.println(" %"); }*/
  
}

void dataTrigger() {
  unsigned long jetzt = micros();               //(1)
  unsigned long dauer = jetzt - letzterTrigger; //(2)
  letzterTrigger = jetzt;                       //(3)

  if (dauer > 30000) { //(4)
    paketNum = 0;
  }

  if (dauer > 7000) {             //(5)
    if (bitPos == 36) {           //(6)
      if (paketNum == 0) {        //(7)
        paketNum = 1;
      } else if (paketNum == 1) { //(8)
        paketDekodieren();
        paketNum = -1;
      }
    }
    bitPos = 0;       //(9)
    return;
  }

  if (paketNum < 0) return;   //(10)
  if (bitPos < 0) return;     //(11)

  if (paketNum == 0) {
    paket1Bits[bitPos] = (dauer > 3300); //(12)
  } else {
    paket2Bits[bitPos] = (dauer > 3300); //(13)
  }
  bitPos++;
  if (bitPos > 36) bitPos = -1;          //(14)
}

void setup() {
  Serial.begin(115200);           //(1)

  pinMode(PIN_ANEMOMETER, INPUT); //(2)
  attachInterrupt(digitalPinToInterrupt(PIN_ANEMOMETER), dataTrigger, FALLING); //(3)

  Serial.println("Starting");     //(4)
}

void loop() {}                    //(5)
