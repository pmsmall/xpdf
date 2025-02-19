//========================================================================
//
// Stream.cc
//
//========================================================================

#pragma implementation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <mem.h>
#include "config.h"
#include "Error.h"
#include "Object.h"
#include "Stream.h"
#include "Stream.CCITT.h"

//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

void Stream::setPos(int pos) {
  error(0, "Internal: called setPos() on non-FileStream");
}

Boolean Stream::checkHeader() {
  Boolean ok;
  char buf[20];
  double version;
  int i;

  ok = true;
  for (i = 0; i < 19; ++i) {
    buf[i] = getChar();
    if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r')
      break;
  }
  buf[i] = '\0';
  if (!strncmp(buf, "%PDF-", 5)) {
    version = atof(&buf[5]);
    if (!(buf[5] >= '0' && buf[5] <= '9') || version > pdfVersionNum) {
      error(0, "PDF version %s -- xpdf supports version %s"
	    " (continuing anyway)", &buf[5], pdfVersion);
      ok = false;
    }
  } else {
    error(0, "May not be a PDF file (continuing anyway)");
    ok = false;
  }
  return ok;
}

//------------------------------------------------------------------------
// FileStream
//------------------------------------------------------------------------

FileStream::FileStream(FILE *f1, int start1, int length1, Object *dict1) {
  f = f1;
  start = start1;
  length = length1;
  pos = start;
  savePos = -1;
  dict = *dict1;
}

FileStream::~FileStream() {
  if (savePos >= 0)
    fseek(f, savePos, SEEK_SET);
  dict.free();
}

void FileStream::reset() {
  savePos = ftell(f);
  fseek(f, start, SEEK_SET);
  pos = start;
}

int FileStream::getChar() {
  int c;

  if (length == 0)
    return EOF;
  if (length > 0)
    --length;
  c = fgetc(f);
  if (c != EOF)
    ++pos;
  return c;
}

void FileStream::setPos(int pos1) {
  if (pos1 >= 0) {
    fseek(f, pos1, SEEK_SET);
    pos = pos1;
  } else {
    fseek(f, pos1, SEEK_END);
    pos = ftell(f);
  }
}

Dict *FileStream::getDict() {
  return dict.getDict();
}

//------------------------------------------------------------------------
// ASCIIHexStream
//------------------------------------------------------------------------

ASCIIHexStream::ASCIIHexStream(Stream *str1) {
  str = str1;
  eof = false;
}

ASCIIHexStream::~ASCIIHexStream() {
  delete str;
}

void ASCIIHexStream::reset() {
  str->reset();
  eof = false;
}

int ASCIIHexStream::getChar() {
  int c1, c2, x;

  if (eof)
    return EOF;
  do {
    c1 = str->getChar();
  } while (isspace(c1));
  if (c1 == '>') {
    eof = true;
    return EOF;
  }
  do {
    c2 = str->getChar();
  } while (isspace(c2));
  if (c2 == '>') {
    eof = true;
    c2 = '0';
  }
  if (c1 >= '0' && c1 <= '9') {
    x = (c1 - '0') << 4;
  } else if (c1 >= 'A' && c1 <= 'F') {
    x = (c1 - 'A' + 10) << 4;
  } else if (c1 >= 'a' && c1 <= 'f') {
    x = (c1 - 'a' + 10) << 4;
  } else {
    error(getPos(), "Illegal character <%02d> in ASCIIHex stream", c1);
    x = 0;
  }
  if (c2 >= '0' && c2 <= '9') {
    x += c2 - '0';
  } else if (c2 >= 'A' && c2 <= 'F') {
    x += c2 - 'A' + 10;
  } else if (c2 >= 'a' && c2 <= 'f') {
    x += c2 - 'a' + 10;
  } else {
    error(getPos(), "Illegal character <%02d> in ASCIIHex stream", c2);
  }
  return x & 0xff;
}

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

ASCII85Stream::ASCII85Stream(Stream *str1) {
  str = str1;
  index = n = 0;
  eof = false;
}

ASCII85Stream::~ASCII85Stream() {
  delete str;
}

void ASCII85Stream::reset() {
  str->reset();
  index = n = 0;
  eof = false;
}

int ASCII85Stream::getChar() {
  int k;
  ulong t;

  if (index >= n && eof)
    return EOF;
  if (index >= n) {
    index = 0;
    do {
      c[0] = str->getChar();
    } while (c[0] == '\n' || c[0] == '\r');
    if (c[0] == '~') {
      eof = true;
      n = 0;
      return EOF;
    } else if (c[0] == 'z') {
      b[0] = b[1] = b[2] = b[3] = 0;
      n = 4;
    } else {
      for (k = 1; k < 5; ++k) {
	do {
	  c[k] = str->getChar();
	} while (c[k] == '\n' || c[k] == '\r');
	if (c[k] == '~')
	  break;
      }
      n = k - 1;
      if (k < 5 && c[k] == '~') {
	for (++k; k < 5; ++k)
	  c[k] = 0x21;
	eof = true;
      }
      t = 0;
      for (k = 0; k < 5; ++k)
	t = t * 85 + (c[k] - 0x21);
      for (k = 3; k >= 0; --k) {
	b[k] = t & 0xff;
	t >>= 8;
      }
    }
  }
  return b[index++];
}

//------------------------------------------------------------------------
// LZWStream
//------------------------------------------------------------------------

LZWStream::LZWStream(Stream *str1, int predictor1, int columns1, int colors1,
		     int bits1, int early1) {
  str = str1;
  predictor = predictor1;
  columns = columns1;
  colors = colors1;
  bits = bits1;
  early = early1;
  eof = false;
  inputBits = 0;
  clearTable();
}

LZWStream::~LZWStream() {
  delete str;
}

void LZWStream::reset() {
  str->reset();
  eof = false;
  inputBits = 0;
  clearTable();
}

int LZWStream::getChar() {
  int code;
  int nextLength;
  int i, j;

  // if at eof just return EOF
  if (eof)
    return EOF;

  // if buffer not empty, return next char
  if (seqIndex < seqLength)
    return seqBuf[seqIndex++];

  // check for eod and clear-table codes
 start:
  code = getCode();
  if (code == EOF || code == 257) {
    eof = true;
    return EOF;
  }
  if (code == 256) {
    clearTable();
    goto start;
  }
  if (nextCode >= 4096) {
    error(getPos(), "Bad LZW stream - expected clear-table code");
    clearTable();
  }

  // process the next code
  nextLength = seqLength + 1;
  if (code < 256) {
    seqBuf[0] = code;
    seqLength = 1;
  } else if (code < nextCode) {
    seqLength = table[code].length;
    for (i = seqLength - 1, j = code; i > 0; --i) {
      seqBuf[i] = table[j].tail;
      j = table[j].head;
    }
    seqBuf[0] = j;
  } else {
    seqBuf[seqLength] = newChar;
    ++seqLength;
  }
  newChar = seqBuf[0];
  if (first) {
    first = false;
  } else {
    table[nextCode].length = nextLength;
    table[nextCode].head = prevCode;
    table[nextCode].tail = newChar;
    ++nextCode;
    if (nextCode + early == 512)
      nextBits = 10;
    else if (nextCode + early == 1024)
      nextBits = 11;
    else if (nextCode + early == 2048)
      nextBits = 12;
  }
  prevCode = code;

  // return char from buffer
  seqIndex = 0;
  return seqBuf[seqIndex++];
}

void LZWStream::clearTable() {
  nextCode = 258;
  nextBits = 9;
  seqIndex = seqLength = 0;
  first = true;
}

int LZWStream::getCode() {
  int c;
  int code;

  while (inputBits < nextBits) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    inputBuf = (inputBuf << 8) | (c & 0xff);
    inputBits += 8;
  }
  code = (inputBuf >> (inputBits - nextBits)) & ((1 << nextBits) - 1);
  inputBits -= nextBits;
  return code;
}

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

CCITTFaxStream::CCITTFaxStream(Stream *str1, int encoding1, Boolean byteAlign1,
			       int columns1, int rows1, Boolean black1) {
  str = str1;
  encoding = encoding1;
  byteAlign = byteAlign1;
  columns = columns1;
  rows = rows1;
  black = black1;
  refLine = (short *)smalloc((columns + 2) * sizeof(short));
  codingLine = (short *)smalloc((columns + 2) * sizeof(short));

  eof = false;
  inputBits = 0;
  codingLine[0] = 0;
  codingLine[1] = refLine[2] = columns;
  a0 = 1;
}

CCITTFaxStream::~CCITTFaxStream() {
  delete str;
  sfree(refLine);
  sfree(codingLine);
}

void CCITTFaxStream::reset() {
  str->reset();
  eof = false;
  inputBits = 0;
  codingLine[0] = 0;
  codingLine[1] = refLine[2] = columns;
  a0 = 1;
}

int CCITTFaxStream::getChar() {
  short code1, code2;
  int ret;
  int bits, i;

  // if at eof just return EOF
  if (eof && codingLine[a0] >= columns)
    return EOF;

  // read the next row
  if (codingLine[a0] >= columns) {
    for (i = 0; codingLine[i] < columns; ++i)
      refLine[i] = codingLine[i];
    refLine[i] = refLine[i + 1] = columns;
    b1 = 1;
    codingLine[a0 = 0] = 0;
    do {
      code1 = getCode(twoDimTable);
      switch (code1) {
      case twoDimPass:
	if (refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimHoriz:
	if ((a0 & 1) == 0) {
	  if ((code1 = getCode(whiteTable)) >= 64)
	    code1 += getCode(whiteTable);
	  if ((code2 = getCode(blackTable)) >= 64)
	    code2 += getCode(blackTable);
	} else {
	  if ((code1 = getCode(blackTable)) >= 64)
	    code1 += getCode(blackTable);
	  if ((code2 = getCode(whiteTable)) >= 64)
	    code2 += getCode(whiteTable);
	}
	codingLine[a0 + 1] = codingLine[a0] + code1;
	++a0;
	codingLine[a0 + 1] = codingLine[a0] + code2;
	++a0;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVert0:
	codingLine[++a0] = refLine[b1];
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertR1:
	codingLine[++a0] = refLine[b1] + 1;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL1:
	codingLine[++a0] = refLine[b1] - 1;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVertR2:
	codingLine[++a0] = refLine[b1] + 2;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL2:
	codingLine[++a0] = refLine[b1] - 2;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVertR3:
	codingLine[++a0] = refLine[b1] + 3;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL3:
	codingLine[++a0] = refLine[b1] - 3;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case ccittEOL:
	break;
      case EOF:
	eof = true;
	codingLine[a0 = 0] = columns;
	return EOF;
      default:
	error(0, "Bad 2D code %04x in CCITTFax stream", code1);
	return EOF;
      }
    } while (codingLine[a0] < columns);
    if (codingLine[a0] != columns)
      error(0, "CCITTFax row is wrong length (%d)", codingLine[a0]);
    a0 = 0;
    outputBits = codingLine[1] - codingLine[0];
  }

  // get a byte
  bits = 0;
  ret = 0;
  do {
    if (outputBits > 8 - bits)
      i = 8 - bits;
    else
      i = outputBits;
    ret = ret << i;
    if (a0 & 1)
      ret |= 0xff >> (8 - i);
    bits += i;
    if ((outputBits -= i) == 0) {
      ++a0;
      if (codingLine[a0] < columns)
	outputBits = codingLine[a0 + 1] - codingLine[a0];
    }
  } while (bits < 8 && codingLine[a0] < columns);
  if (bits < 8)
    ret <<= 8 - bits;
  return black ? ~ret : ret;
}

short CCITTFaxStream::getCode(CCITTCodeTable *table) {
  short code;
  int codeBits, bit;
  int a, b, m;

  code = 0;
  codeBits = 0;
  do {
    // add a bit to the code
    if ((bit = getBit()) == EOF)
      return EOF;
    code = (code << 1) + bit;
    ++codeBits;

    // search code table
    // invariant: codes[a].code < code < codes[b].code
    if (table[codeBits].numCodes > 0) {
      a = -1;
      b = table[codeBits].numCodes;
      m = 0;
      while (b - a > 1) {
	m = (a + b) / 2;
	if (table[codeBits].codes[m].code < code)
	  a = m;
	else if (table[codeBits].codes[m].code > code)
	  b = m;
	else
	  a = b = m;
      }
      if (table[codeBits].codes[m].code == code)
	return table[codeBits].codes[m].n;
    }
  } while (codeBits < ccittMaxCodeLen);

  error(0, "Bad code (%04x) in CCITTFax stream", code);
  return EOF;
}

int CCITTFaxStream::getBit() {
  int bit;
  int c;

  if (inputBits == 0) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    inputBuf = c;
    inputBits = 8;
  }
  bit = (inputBuf >> (inputBits - 1)) & 1;
  --inputBits;
  return bit;
}
