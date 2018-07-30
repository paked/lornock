// TODO(harrison): adjust this API interface to take a 'max len', and then use standard NULL termination to denote the end of the string.
// TODO(harrison): properly increment upTo (+1) so that we don't have to outside the function
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

  paramName[0] = '\0';
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

void eatUntilWhitespace(char* out, uint32 outLen, char* line, uint32 lineLen, uint32* head) {
  uint32 i = 0;

  out[i] = '\0';

  while (i + *head < lineLen) {
    ensure(i < outLen);

    out[i] = '\0';

    char c = line[i + *head];

    if (isspace(c)) {
      break;
    }

    out[i] = c;

    i += 1;
  }

  *head = *head + i;
}
