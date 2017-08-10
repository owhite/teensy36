void slidesToStore(uint16_t row) {
  slideStore.slideData.slideVal1 = slideVals[0];
  slideStore.slideData.slideVal2 = slideVals[1];
  slideStore.slideData.slideVal3 = slideVals[2];
  slideStore.slideData.slideVal4 = slideVals[3];

  for(uint8_t i = 0; i < SLIDE_PACKET_SIZE; i++) {
    slideArray[row][i] = slideStore.slidePacket[i];
  }
}

void storeToSlides(uint16_t row) {
  for(uint8_t i = 0; i < SLIDE_PACKET_SIZE; i++) {
    slideStore.slidePacket[i] = slideArray[row][i];
  }

  slideVals[0] = slideStore.slideData.slideVal1;
  slideVals[1] = slideStore.slideData.slideVal2;
  slideVals[2] = slideStore.slideData.slideVal3;
  slideVals[3] = slideStore.slideData.slideVal4;
}

void loadPositions(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t x4) {
  moveToPositions[0] = x1;
  moveToPositions[1] = x2;
  moveToPositions[2] = x3;
  moveToPositions[3] = x4;
}

