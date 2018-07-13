bool getLine(char* line, uint32* lineLen, uint32* upTo, char* source, uint32 sourceLen) {
  line[0] = '\0';

  uint32 i = 0;

  while (i + *upTo < sourceLen) {
    line[i] = '\0';

    char c = source[i + *upTo];

    if (c == '\n') {
      break;
    }

    line[i] = c;

    i++;
  }

  *lineLen = i;
  *upTo = i + *upTo;

  return i > 0;
}

void eatUntilChar(char* paramName, uint32 len, char to, char* line, uint32 lineLen, uint32* upTo) {
  uint32 i = 0;

  while (i + *upTo < lineLen) {
    paramName[i] = '\0';

    char c = line[i + *upTo];

    if (i > len) {
      logln("WARNING: eatUntilChar buffer not big enough");

      break;
    }

    if (c == to) {
      break;
    }

    paramName[i] = c;

    i++;
  }

  *upTo = *upTo + i;
}
