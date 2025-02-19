//========================================================================
//
// Stream.cc
//
// Copyright 1996 Derek B. Noonburg
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

Stream *Stream::addFilters(Object *dict) {
  Object obj, obj2;
  Object params, params2;
  Stream *str;
  int i;

  str = this;
  dict->dictLookup("Filter", &obj);
  if (obj.isNull()) {
    obj.free();
    dict->dictLookup("F", &obj);
  }
  dict->dictLookup("DecodeParms", &params);
  if (params.isNull()) {
    params.free();
    dict->dictLookup("DP", &params);
  }
  if (obj.isName()) {
    str = makeFilter(obj.getName(), str, &params);
  } else if (obj.isArray()) {
    for (i = 0; i < obj.arrayGetLength(); ++i) {
      obj.arrayGet(i, &obj2);
      if (params.isArray())
	params.arrayGet(i, &params2);
      else
	params2.initNull();
      if (obj2.isName())
	str = makeFilter(obj2.getName(), str, &params2);
      else
	error(getPos(), "Bad filter name");
      obj2.free();
      params2.free();
    }
  } else if (!obj.isNull()) {
    error(getPos(), "Bad 'Filter' attribute in stream");
  }
  obj.free();
  params.free();

  return str;
}

Stream *Stream::makeFilter(char *name, Stream *str, Object *params) {
  int predictor;		// parameters
  int colors;
  int bits;
  int early;
  int encoding;
  Boolean byteAlign;
  Boolean black;
  int columns, rows;
  Object obj;

  if (!strcmp(name, "ASCIIHexDecode") || !strcmp(name, "AHx")) {
    str = new ASCIIHexStream(str);
  } else if (!strcmp(name, "ASCII85Decode") || !strcmp(name, "A85")) {
    str = new ASCII85Stream(str);
  } else if (!strcmp(name, "LZWDecode") || !strcmp(name, "LZW")) {
    predictor = 1;
    columns = 1;
    colors = 1;
    bits = 8;
    early = 1;
    if (params->isDict()) {
      params->dictLookup("Predictor", &obj);
      if (obj.isInt())
	predictor = obj.getInt();
      obj.free();
      params->dictLookup("Columns", &obj);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Colors", &obj);
      if (obj.isInt())
	colors = obj.getInt();
      obj.free();
      params->dictLookup("BitsPerComponent", &obj);
      if (obj.isInt())
	bits = obj.getInt();
      obj.free();
      params->dictLookup("EarlyChange", &obj);
      if (obj.isInt())
	early = obj.getInt();
      obj.free();
    }
    str = new LZWStream(str, predictor, columns, colors, bits, early);
//~  } else if (!strcmp(name, "RunLengthDecode")) {
  } else if (!strcmp(name, "CCITTFaxDecode")) {
    encoding = 0;
    byteAlign = false;
    columns = 1728;
    rows = 0;
    black = false;
    if (params->isDict()) {
      params->dictLookup("K", &obj);
      if (obj.isInt())
	encoding = obj.getInt();
      obj.free();
      params->dictLookup("EncodedByteAlign", &obj);
      if (obj.isBool())
	byteAlign = obj.getBool();
      obj.free();
      params->dictLookup("Columns", &obj);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Rows", &obj);
      if (obj.isInt())
	rows = obj.getInt();
      obj.free();
      params->dictLookup("BlackIs1", &obj);
      if (obj.isBool())
	black = obj.getBool();
      obj.free();
    }
    str = new CCITTFaxStream(str, encoding, byteAlign, columns, rows, black);
  } else if (!strcmp(name, "DCTDecode")) {
    str = new DCTStream(str);
  } else {
    error(getPos(), "Unknown filter '%s'", name);
  }
  return str;
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

//------------------------------------------------------------------------
// SubStream
//------------------------------------------------------------------------

SubStream::SubStream(Stream *str1, Object *dict1) {
  str = str1;
  dict = *dict1;
}

SubStream::~SubStream() {
  dict.free();
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
  zPipe = NULL;
}

LZWStream::~LZWStream() {
  if (zPipe) {
    pclose(zPipe);
    zPipe = NULL;
    unlink(zName);
  }
  delete str;
}

void LZWStream::reset() {
  FILE *f;

  str->reset();
  if (zPipe) {
    pclose(zPipe);
    zPipe = NULL;
    unlink(zName);
  }
  strcpy(zCmd, uncompressCmd);
  strcat(zCmd, " ");
  zName = zCmd + strlen(zCmd);
  tmpnam(zName);
  strcat(zName, ".Z");
  if (!(f = fopen(zName, "w"))) {
    error(getPos(), "Couldn't open temporary file '%s'", zName);
    return;
  }
  dumpFile(f);
  fclose(f);
  if (!(zPipe = popen(zCmd, "r"))) {
    error(getPos(), "Couldn't popen '%s'", zCmd);
    unlink(zName);
    return;
  }
}

void LZWStream::dumpFile(FILE *f) {
  int outCodeBits;		// size of output code
  int outBits;			// max output code
  int outBuf[8];		// output buffer
  int outData;			// temporary output buffer
  int inCode, outCode;		// input and output codes
  int nextCode;			// next code index
  Boolean eof;			// set when EOF is reached
  Boolean clear;		// set if table needs to be cleared
  Boolean first;		// indicates first code word after clear
  int i, j;

  // magic number
  fputc(0x1f, f);
  fputc(0x9d, f);

  // max code length, block mode flag
  fputc(0x8c, f);

  // init input side
  inCodeBits = 9;
  inputBuf = 0;
  inputBits = 0;
  eof = false;

  // init output side
  outCodeBits = 9;

  // clear table
  first = true;
  nextCode = 258;

  clear = false;
  do {
    for (i = 0; i < 8; ++i) {
      // check for table overflow
      if (nextCode + early > 0x1001) {
	inCode = 256;

      // read input code
      } else {
	do {
	  inCode = getCode();
	  if (inCode == EOF) {
	    eof = true;
	    inCode = 0;
	  }
	} while (first && inCode == 256);
      }

      // compute output code
      if (inCode < 256) {
	outCode = inCode;
      } else if (inCode == 256) {
	outCode = 256;
	clear = true;
      } else if (inCode == 257) {
	outCode = 0;
	eof = true;
      } else {
	outCode = inCode - 1;
      }
      outBuf[i] = outCode;

      // next code index
      if (first)
	first = false;
      else
	++nextCode;

      // check input code size
      if (nextCode + early == 0x200)
	inCodeBits = 10;
      else if (nextCode + early == 0x400) {
	inCodeBits = 11;
      } else if (nextCode + early == 0x800) {
	inCodeBits = 12;
      }

      // check for eof/clear
      if (eof)
	break;
      if (clear) {
	i = 8;
	break;
      }
    }

    // write output block
    outData = 0;
    outBits = 0;
    j = 0;
    while (j < i || outBits > 0) {
      if (outBits < 8 && j < i) {
	outData = outData | (outBuf[j++] << outBits);
	outBits += outCodeBits;
      }
      fputc(outData & 0xff, f);
      outData >>= 8;
      outBits -= 8;
    }

    // check output code size
    if (nextCode - 1 == 512 ||
	nextCode - 1 == 1024 ||
	nextCode - 1 == 2048 ||
	nextCode - 1 == 4096) {
      outCodeBits = inCodeBits;
    }

    // clear table if necessary
    if (clear) {
      inCodeBits = 9;
      outCodeBits = 9;
      first = true;
      nextCode = 258;
      clear = false;
    }
  } while (!eof);
}

int LZWStream::getCode() {
  int c;
  int code;

  while (inputBits < inCodeBits) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    inputBuf = (inputBuf << 8) | (c & 0xff);
    inputBits += 8;
  }
  code = (inputBuf >> (inputBits - inCodeBits)) & ((1 << inCodeBits) - 1);
  inputBits -= inCodeBits;
  return code;
}

int LZWStream::getChar() {
  int c;

  if (!zPipe)
    return EOF;
  if ((c = fgetc(zPipe)) == EOF) {
    pclose(zPipe);
    zPipe = NULL;
    unlink(zName);
  }
  return c;
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
  if (outputBits >= 8) {
    ret = ((a0 & 1) == 0) ? 0xff : 0x00;
    if ((outputBits -= 8) == 0) {
      ++a0;
      if (codingLine[a0] < columns)
	outputBits = codingLine[a0 + 1] - codingLine[a0];
    }
  } else {
    bits = 8;
    ret = 0;
    do {
      if (outputBits > bits) {
	i = bits;
	bits = 0;
	if ((a0 & 1) == 0)
	  ret |= 0xff >> (8 - i);
	outputBits -= i;
      } else {
	i = outputBits;
	bits -= outputBits;
	if ((a0 & 1) == 0)
	  ret |= (0xff >> (8 - i)) << bits;
	outputBits = 0;
	++a0;
	if (codingLine[a0] < columns)
	  outputBits = codingLine[a0 + 1] - codingLine[a0];
      }
    } while (bits > 0 && codingLine[a0] < columns);
  }
  return black ? (ret ^ 0xff) : ret;
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

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

DCTStream::DCTStream(Stream *str1) {
  str = str1;
}

DCTStream::~DCTStream() {
  delete str;
}

void DCTStream::reset() {
  str->reset();
}

int DCTStream::getChar() {
  return 0;
}
