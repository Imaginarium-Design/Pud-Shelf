#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 5
#define NUM_OF_READERS 4
#define RELAY_PIN 2

byte ACCEPTED_TAGS_A[][4] = {
  {0xA9, 0x46, 0x22, 0x56}, 
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ACCEPTED_TAGS_B[][4] = {
  {0x59, 0x1F, 0x41, 0x56},
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ACCEPTED_TAGS_C[][4] = {
  {0x49, 0x8c, 0xFC, 0x51},
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ACCEPTED_TAGS_D[][4] = {
  {0X49, 0X2B, 0XFB, 0X51},
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ACCEPTED_TAGS_E[][4] = {
  {0X49, 0X2B, 0XFB, 0X51},
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ACCEPTED_TAGS_F[][4] = {
  {0X49, 0X2B, 0XFB, 0X51},
  {0xA9, 0x0B, 0x3F, 0x56}
};

byte ssPins[] = { 10, 8, 7, 6 };

bool ValidTag[NUM_OF_READERS];
bool TagPresent[NUM_OF_READERS];
bool TagPresentPrev[NUM_OF_READERS];
int ErrorCounter[NUM_OF_READERS];
bool TagFound[NUM_OF_READERS];

MFRC522 mfrc522[NUM_OF_READERS];

void setup() {
  // Attempt to open the Serial Port
  Serial.begin(9600);
  SPI.begin();

  // Set up the Relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Initiate the properties and instances of each RFID
  for (int i = 0; i < NUM_OF_READERS; i++){
    ValidTag[i] = false;
    TagPresent[i] = false;
    TagPresentPrev[i] = false;
    ErrorCounter[i] = 0;
    TagFound[i] = 0;

    // Initialise the reader and set gain to max (optional)
    mfrc522[i].PCD_Init(ssPins[i], RST_PIN);
    //mfrc522[i].PCD_WriteRegister(mfrc522[i].RFCfgReg, (0x07<<4));
    //mfrc522[i].PCD_AntennaOn();

    // Print to Serial if it's connected
    if (Serial){
      Serial.print("Reader ");
      Serial.print(i + 1);
      Serial.print(": ");
      mfrc522[i].PCD_DumpVersionToSerial();
    }
  }

  if (Serial)
    Serial.println("Setup Complete!");
}

void loop() {
  // Cycle through each RFID Reader
  for (int i = 0; i < NUM_OF_READERS; i++){
    // Was a tag previously present?
    TagPresentPrev[i] = TagPresent[i];

    // Error Counter (just in case!)
    ErrorCounter[i]++;
    if (ErrorCounter[i] > 2)
      TagFound[i] = false;

    // Blank the register
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    mfrc522[i].PCD_WriteRegister(mfrc522[i].TxModeReg, 0x00);
    mfrc522[i].PCD_WriteRegister(mfrc522[i].RxModeReg, 0x00);
    mfrc522[i].PCD_WriteRegister(mfrc522[i].ModWidthReg, 0x26);

    // Is the status of the RFID OK?
    MFRC522::StatusCode result = mfrc522[i].PICC_RequestA(bufferATQA, &bufferSize);
    if (result == mfrc522[i].STATUS_OK){
      // If it can't read the card Serial again, we can quit out.
      if (!mfrc522[i].PICC_ReadCardSerial()){
        break;
      }

      // No error, so set the counter to 0 and indicate a tag was found
      ErrorCounter[i] = 0;
      TagFound[i] = true;
    }

    // A tag is present if a tag was found!
    TagPresent[i] = TagFound[i];
    
    // If a tag wasn't present before, but is now...
    if (TagPresent[i] && !TagPresentPrev[i]){
      if (Serial){
        Serial.print("Reader ");
        Serial.print(i + 1);
        Serial.println(" Tag Found!");
      }

      // Set whether this is a valid tag ID or not
      ValidTag[i] = CheckTag(i);
    }

     // If a tag was present but now isn't...
    if (!TagPresent[i] && TagPresentPrev[i]){
      if (Serial){
        Serial.print("Reader ");
        Serial.print(i + 1);
        Serial.println(" Tag Removed!");
      }

      // Set it as an invalid tag
      ValidTag[i] = false;
    }
  }

  // Once we've checked all the readers,
  // check to see if all readers have a tag and it is valid.
  bool bCompleted = true;
  for(int i = 0; i < NUM_OF_READERS; i++){
    bCompleted = TagPresent[i] && ValidTag[i];

    // If one is false, we can break the loop
    if (!bCompleted)
      break;
  }

  // Set the relay to the correct position.
  FlickRelay(bCompleted);
}

bool CheckTag(int readerNum){
  MFRC522 mfrc = mfrc522[readerNum];
  bool bMatch = true;
  
  // Cycle through each accepted Tag ID
  for (int x = 0; x < 2; x++) {
    bMatch = true;
    
    // Check each chunk of the ID
    for (int y = 0; y < mfrc.uid.size; y++){
      switch (readerNum){
        case 0:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_A[x][y])
            bMatch = false;
          break;
        case 1:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_B[x][y])
            bMatch = false;
          break;
        case 2:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_C[x][y])
            bMatch = false;
          break;
        case 3:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_D[x][y])
            bMatch = false;
          break;
        case 4:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_E[x][y])
            bMatch = false;
          break;
        case 5:
          if (mfrc.uid.uidByte[y] != ACCEPTED_TAGS_F[x][y])
            bMatch  = false;
          break;
      }

      // If we don't have a match for one segment, break out to next ID
      if (!bMatch)
        break;
    }

    // If we have a full match, break out of the loop;
    if (bMatch)
      break;
  }

  if (Serial){
    Serial.print("Tag Accepted: ");
    Serial.println(bMatch ? "Yes" : "No");
  }
  
  return bMatch;
}

void FlickRelay(bool bOpen){
  digitalWrite(RELAY_PIN, bOpen ? LOW : HIGH);

  // If we've opened the relay, delay for a few seconds.
  if (bOpen)
    delay(3300);
}
