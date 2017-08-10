void getRandomFileName(char *name) { 
  name[0] = SD_FILE_KEY;  name[1] = SD_FILE_KEY;  name[2] = SD_FILE_KEY;
  name[3] = random(26) + 65; name[4] = random(26) + 65; name[5] = random(26) + 65;
  name[6] = '.';  name[7] = 't';  name[8] = 'x';  name[9] = 't';
  name[10] = '\0';
}

void randomFileSelect(char *name) {
  uint8_t total = 0;

  // volume working directory, vwd, is root.
  sd.vwd()->rewind();
  while (file.openNext(sd.vwd(), O_READ)) {
    file.getName(name, SD_FILE_NAME_SIZE);
    if (name[0] == SD_FILE_KEY && name[1] == SD_FILE_KEY  && name[2] == SD_FILE_KEY ) {
      Serial.printf("name: %s\n", name);
      total++;
    }

    file.close();
  }

  uint8_t i = 0;
  total = random(total);

  sd.vwd()->rewind();
  while (file.openNext(sd.vwd(), O_READ)) {
    file.getName(name, SD_FILE_NAME_SIZE);
    if (name[0] == SD_FILE_KEY && name[1] == SD_FILE_KEY && name[2] == SD_FILE_KEY) {
      if (i == total) {  file.close(); break; }
      i++;
    }
    file.close();
  }
}

void writeSDFile(char *name, uint16_t rows) {
  file = sd.open(name, O_CREAT | O_WRITE);

  if (!file) { errorHalt("SD write failed"); }

  for (uint16_t i = 0; i < rows; i++) {
    for (uint8_t j = 0; j < SLIDE_PACKET_SIZE; j++) {
      file.print((char) slideArray[i][j]);
    }
    file.println();
  }

  file.close();
}

uint16_t readSDFile(char *file) {
  uint8_t ch;
  uint8_t i = 0;
  uint16_t rows = 0;

  char buffer[SLIDE_PACKET_SIZE+1];

 ifstream sdin(file);

  while (sdin.getline(buffer, SLIDE_PACKET_SIZE + 1, '\n') || sdin.gcount()) {
    if (sdin.fail()) {
      Serial.print("Partial long line");
      sdin.clear(sdin.rdstate() & ~ios_base::failbit);
    } else if (sdin.eof()) {
      Serial.print("Partial final line");  // sdin.fail() is false
    } else {
      for (uint8_t j = 0; j < SLIDE_PACKET_SIZE; j++) {
	slideArray[rows][j] = buffer[j];
      }
      rows++;
    }
  }

  return(rows);
}

void dumpArray() {
  for (uint16_t i = 0; i < 20; i++) {
    for (uint8_t j = 0; j < SLIDE_PACKET_SIZE; j++) {
      slideStore.slidePacket[j] = slideArray[i][j];
    }
    Serial.printf("%d %d %d %d\n",
		  slideStore.slideData.slideVal1,
		  slideStore.slideData.slideVal2,
		  slideStore.slideData.slideVal3,
		  slideStore.slideData.slideVal4);

  }      
}

void clearArray() {
  for (uint16_t i = 0; i < 20; i++) {
    for (uint16_t j = 0; j < SLIDE_PACKET_SIZE; j++) {
      slideArray[i][j] = 0;
    }
  }
}

