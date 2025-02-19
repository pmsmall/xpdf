//========================================================================
//
// Stream.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <gmem.h>
#include "config.h"
#include "Error.h"
#include "Object.h"
#include "Stream.h"
#include "Stream-CCITT.h"

#ifdef VMS
extern "C" int unlink(char *filename);
#ifdef __GNUC__
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#endif

//------------------------------------------------------------------------

#define headerSearchSize 1024	// read this many bytes at beginning of
				//   file to look for '%PDF'
//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

void Stream::setPos(int pos) {
  error(0, "Internal: called setPos() on non-FileStream");
}

GString *Stream::getPSFilter(char *indent) {
  return new GString();
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
  GBool byteAlign;
  GBool black;
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
    byteAlign = gFalse;
    columns = 1728;
    rows = 0;
    black = gFalse;
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

  if (length >= 0 && pos >= start + length)
    return EOF;
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

GBool FileStream::checkHeader() {
  char buf[headerSearchSize+1];
  char *p;
  double version;
  int i;

  for (i = 0; i < headerSearchSize; ++i)
    buf[i] = getChar();
  buf[headerSearchSize] = '\0';
  for (i = 0; i < headerSearchSize - 5; ++i) {
    if (!strncmp(&buf[i], "%PDF-", 5))
      break;
  }
  if (i >= headerSearchSize - 5) {
    error(0, "May not be a PDF file (continuing anyway)");
    return gFalse;
  }
  start += i;
  p = strtok(&buf[i+5], " \t\n\r");
  version = atof(p);
  if (!(buf[i+5] >= '0' && buf[i+5] <= '9') || version > pdfVersionNum) {
    error(getPos(), "PDF version %s -- xpdf supports version %s"
	  " (continuing anyway)", p, pdfVersion);
    return gFalse;
  }
  return gTrue;
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
  eof = gFalse;
}

ASCIIHexStream::~ASCIIHexStream() {
  delete str;
}

void ASCIIHexStream::reset() {
  str->reset();
  eof = gFalse;
}

int ASCIIHexStream::getChar() {
  int c1, c2, x;

  if (eof)
    return EOF;
  do {
    c1 = str->getChar();
  } while (isspace(c1));
  if (c1 == '>') {
    eof = gTrue;
    return EOF;
  }
  do {
    c2 = str->getChar();
  } while (isspace(c2));
  if (c2 == '>') {
    eof = gTrue;
    c2 = '0';
  }
  if (c1 >= '0' && c1 <= '9') {
    x = (c1 - '0') << 4;
  } else if (c1 >= 'A' && c1 <= 'F') {
    x = (c1 - 'A' + 10) << 4;
  } else if (c1 >= 'a' && c1 <= 'f') {
    x = (c1 - 'a' + 10) << 4;
  } else {
    error(getPos(), "Illegal character <%02x> in ASCIIHex stream", c1);
    x = 0;
  }
  if (c2 >= '0' && c2 <= '9') {
    x += c2 - '0';
  } else if (c2 >= 'A' && c2 <= 'F') {
    x += c2 - 'A' + 10;
  } else if (c2 >= 'a' && c2 <= 'f') {
    x += c2 - 'a' + 10;
  } else {
    error(getPos(), "Illegal character <%02x> in ASCIIHex stream", c2);
  }
  return x & 0xff;
}

GString *ASCIIHexStream::getPSFilter(char *indent) {
  GString *s;

  s = str->getPSFilter(indent);
  s->append(indent)->append("/ASCIIHexDecode filter\n");
  return s;
}

GBool ASCIIHexStream::isBinary(GBool last) {
  return str->isBinary(gFalse);
}

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

ASCII85Stream::ASCII85Stream(Stream *str1) {
  str = str1;
  index = n = 0;
  eof = gFalse;
}

ASCII85Stream::~ASCII85Stream() {
  delete str;
}

void ASCII85Stream::reset() {
  str->reset();
  index = n = 0;
  eof = gFalse;
}

int ASCII85Stream::getChar() {
  int k;
  Gulong t;

  if (index >= n) {
    if (eof)
      return EOF;
    index = 0;
    do {
      c[0] = str->getChar();
    } while (c[0] == '\n' || c[0] == '\r');
    if (c[0] == '~') {
      eof = gTrue;
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
	eof = gTrue;
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

GString *ASCII85Stream::getPSFilter(char *indent) {
  GString *s;

  s = str->getPSFilter(indent);
  s->append(indent)->append("/ASCII85Decode filter\n");
  return s;
}

GBool ASCII85Stream::isBinary(GBool last) {
  return str->isBinary(gFalse);
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
#ifdef NO_POPEN
    fclose(zPipe);
#else
    pclose(zPipe);
#endif
    zPipe = NULL;
    unlink(zName);
  }
  delete str;
}

void LZWStream::reset() {
  FILE *f;

  str->reset();
  if (zPipe) {
#ifdef NO_POPEN
    fclose(zPipe);
#else
    pclose(zPipe);
#endif
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
#ifdef NO_POPEN
#ifdef VMS
  if (!system(zCmd)) {
#else
  if (system(zCmd)) {
#endif
    error(getPos(), "Couldn't execute '%s'", zCmd);
    unlink(zName);
    return;
  }
  zName[strlen(zName) - 2] = '\0';
  if (!(zPipe = fopen(zName, "r"))) {
    error(getPos(), "Couldn't open uncompress file '%s'", zName);
    unlink(zName);
    return;
  }
#else
  if (!(zPipe = popen(zCmd, "r"))) {
    error(getPos(), "Couldn't popen '%s'", zCmd);
    unlink(zName);
    return;
  }
#endif
}

void LZWStream::dumpFile(FILE *f) {
  int outCodeBits;		// size of output code
  int outBits;			// max output code
  int outBuf[8];		// output buffer
  int outData;			// temporary output buffer
  int inCode, outCode;		// input and output codes
  int nextCode;			// next code index
  GBool eof;			// set when EOF is reached
  GBool clear;			// set if table needs to be cleared
  GBool first;			// indicates first code word after clear
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
  eof = gFalse;

  // init output side
  outCodeBits = 9;

  // clear table
  first = gTrue;
  nextCode = 258;

  clear = gFalse;
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
	    eof = gTrue;
	    inCode = 0;
	  }
	} while (first && inCode == 256);
      }

      // compute output code
      if (inCode < 256) {
	outCode = inCode;
      } else if (inCode == 256) {
	outCode = 256;
	clear = gTrue;
      } else if (inCode == 257) {
	outCode = 0;
	eof = gTrue;
      } else {
	outCode = inCode - 1;
      }
      outBuf[i] = outCode;

      // next code index
      if (first)
	first = gFalse;
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
      first = gTrue;
      nextCode = 258;
      clear = gFalse;
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
#ifdef NO_POPEN
    fclose(zPipe);
#else
    pclose(zPipe);
#endif
    zPipe = NULL;
    unlink(zName);
  }
  return c;
}

GString *LZWStream::getPSFilter(char *indent) {
  GString *s;

  s = str->getPSFilter(indent);
  s->append(indent)->append("/LZWDecode filter\n");
  return s;
}

GBool LZWStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

CCITTFaxStream::CCITTFaxStream(Stream *str1, int encoding1, GBool byteAlign1,
			       int columns1, int rows1, GBool black1) {
  str = str1;
  encoding = encoding1;
  byteAlign = byteAlign1;
  columns = columns1;
  rows = rows1;
  black = black1;
  refLine = (short *)gmalloc((columns + 2) * sizeof(short));
  codingLine = (short *)gmalloc((columns + 2) * sizeof(short));

  eof = gFalse;
  inputBits = 0;
  codingLine[0] = 0;
  codingLine[1] = refLine[2] = columns;
  a0 = 1;
}

CCITTFaxStream::~CCITTFaxStream() {
  delete str;
  gfree(refLine);
  gfree(codingLine);
}

void CCITTFaxStream::reset() {
  str->reset();
  eof = gFalse;
  inputBits = 0;
  codingLine[0] = 0;
  codingLine[1] = refLine[2] = columns;
  a0 = 1;
}

int CCITTFaxStream::getChar() {
  short code1, code2;
  int a0New;
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
    a0New = codingLine[a0 = 0] = 0;
    do {
      code1 = getCode(twoDimTable);
      switch (code1) {
      case twoDimPass:
	if (refLine[b1] < columns) {
	  a0New = refLine[b1 + 1];
	  b1 += 2;
	}
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
	codingLine[a0 + 1] = a0New + code1;
	++a0;
	a0New = codingLine[a0 + 1] = codingLine[a0] + code2;
	++a0;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVert0:
	a0New = codingLine[++a0] = refLine[b1];
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertR1:
	a0New = codingLine[++a0] = refLine[b1] + 1;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL1:
	a0New = codingLine[++a0] = refLine[b1] - 1;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVertR2:
	a0New = codingLine[++a0] = refLine[b1] + 2;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL2:
	a0New = codingLine[++a0] = refLine[b1] - 2;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case twoDimVertR3:
	a0New = codingLine[++a0] = refLine[b1] + 3;
	if (refLine[b1] < columns) {
	  ++b1;
	  while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	    b1 += 2;
	}
	break;
      case twoDimVertL3:
	a0New = codingLine[++a0] = refLine[b1] - 3;
	--b1;
	while (refLine[b1] <= codingLine[a0] && refLine[b1] < columns)
	  b1 += 2;
	break;
      case ccittEOL:
	break;
      case EOF:
	eof = gTrue;
	codingLine[a0 = 0] = columns;
	return EOF;
      default:
	error(getPos(), "Bad 2D code %04x in CCITTFax stream", code1);
	return EOF;
      }
    } while (codingLine[a0] < columns);
    if (codingLine[a0] != columns)
      error(getPos(), "CCITTFax row is wrong length (%d)", codingLine[a0]);
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

  error(getPos(), "Bad code (%04x) in CCITTFax stream", code);
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

GString *CCITTFaxStream::getPSFilter(char *indent) {
  GString *s;
  char s1[50];

  s = str->getPSFilter(indent);
  s->append(indent)->append("<< ");
  if (encoding != 0) {
    sprintf(s1, "/K %d ", encoding);
    s->append(s1);
  }
  if (byteAlign)
    s->append("/EncodedByteAlign true ");
  sprintf(s1, "/Columns %d ", columns);
  s->append(s1);
  if (rows != 0) {
    sprintf(s1, "/Rows %d ", rows);
    s->append(s1);
  }
  if (black)
    s->append("/BlackIs1 true ");
  s->append(">> /CCITTFaxDecode filter\n");
  return s;
}

GBool CCITTFaxStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

#define dctCos1    0.98078528	// cos(pi/16)
#define dctSin1    0.19509032	// sin(pi/16)
#define dctCos3    0.83146961	// cos(3*pi/16)
#define dctSin3    0.55557023	// sin(3*pi/16)
#define dctCos6    0.38268343	// cos(6*pi/16)
#define dctSin6    0.92387953	// sin(6*pi/16)
#define dctSqrt2   1.41421356	// sqrt(2)
#define dctSqrt1d2 0.70710678	// sqrt(2) / 2

#define dctCrToR   1.4020	// color conversion parameters
#define dctCbToG  -0.3441363
#define dctCrToG  -0.71413636
#define dctCbToB   1.772

static int dctZigZag[64] = {
   0,
   1,  8,
  16,  9,  2,
   3, 10, 17, 24,
  32, 25, 18, 11, 4,
   5, 12, 19, 26, 33, 40,
  48, 41, 34, 27, 20, 13,  6,
   7, 14, 21, 28, 35, 42, 49, 56,
  57, 50, 43, 36, 29, 22, 15,
  23, 30, 37, 44, 51, 58,
  59, 52, 45, 38, 31,
  39, 46, 53, 60,
  61, 54, 47,
  55, 62,
  63
};

DCTStream::DCTStream(Stream *str1) {
  int i, j;

  str = str1;
  width = height = 0;
  mcuWidth = mcuHeight = 0;
  numComps = 0;
  comp = 0;
  x = y = dy = 0;
  for (i = 0; i < 4; ++i)
    for (j = 0; j < 32; ++j)
      rowBuf[i][j] = NULL;
}

DCTStream::~DCTStream() {
  int i, j;

  delete str;
  for (i = 0; i < numComps; ++i)
    for (j = 0; j < mcuHeight; ++j)
      gfree(rowBuf[i][j]);
}

void DCTStream::reset() {
  str->reset();
  if (!readHeader()) {
    y = height;
    return;
  }
  restartMarker = 0xd0;
  restart();
}

int DCTStream::getChar() {
  int c;

  if (y >= height)
    return EOF;
  if (dy >= mcuHeight) {
    if (!readMCURow()) {
      y = height;
      return EOF;
    }
    comp = 0;
    x = 0;
    dy = 0;
  }
  c = rowBuf[comp][dy][x];
  if (++comp == numComps) {
    comp = 0;
    if (++x == width) {
      x = 0;
      ++y;
      ++dy;
    }
  }
  if (y == height)
    readTrailer();
  return c;
}

void DCTStream::restart() {
  int i;

  inputBits = 0;
  restartCtr = restartInterval;
  for (i = 0; i < numComps; ++i)
    compInfo[i].prevDC = 0;
}

GBool DCTStream::readMCURow() {
  Guchar data[64];
  double pY, pCb, pCr, pR, pG, pB;
  int h, v, horiz, vert, hSub, vSub;
  int x1, x2, y2, x3, y3, x4, y4, x5, y5, comp, i;
  int c;

  for (x1 = 0; x1 < width; x1 += mcuWidth) {

    // deal with restart marker
    if (restartInterval > 0 && restartCtr == 0) {
      c = readMarker();
      if (c != restartMarker) {
	error(getPos(), "Bad DCT data: incorrect restart marker");
	return gFalse;
      }
      if (++restartMarker == 0xd8)
	restartMarker = 0xd0;
      restart();
    }

    // read one MCU
    for (comp = 0; comp < numComps; ++comp) {
      h = compInfo[comp].hSample;
      v = compInfo[comp].vSample;
      horiz = mcuWidth / h;
      vert = mcuHeight / v;
      hSub = horiz / 8;
      vSub = vert / 8;
      for (y2 = 0; y2 < mcuHeight; y2 += vert) {
	for (x2 = 0; x2 < mcuWidth; x2 += horiz) {
	  if (!readDataUnit(&dcHuffTables[compInfo[comp].dcHuffTable],
			    &acHuffTables[compInfo[comp].acHuffTable],
			    quantTables[compInfo[comp].quantTable],
			    &compInfo[comp].prevDC,
			    data))
	    return gFalse;
	  if (hSub == 1 && vSub == 1) {
	    i = 0;
	    for (y3 = 0; y3 < 8; ++y3)
	      for (x3 = 0; x3 < 8; ++x3)
		rowBuf[comp][y2+y3][x1+x2+x3] = data[i++];
	  } else {
	    i = 0;
	    for (y3 = 0, y4 = 0; y3 < 8; ++y3, y4 += vSub) {
	      for (x3 = 0, x4 = 0; x3 < 8; ++x3, x4 += hSub) {
		for (y5 = 0; y5 < vSub; ++y5)
		  for (x5 = 0; x5 < hSub; ++x5)
		    rowBuf[comp][y2+y4+y5][x1+x2+x4+x5] = data[i];
		++i;
	      }
	    }
	  }
	}
      }
    }
    --restartCtr;

    // convert YCbCr to RGB
    if (colorXform && (numComps == 3 || numComps == 4)) {
      for (y2 = 0; y2 < mcuHeight; ++y2) {
	for (x2 = 0; x2 < mcuWidth; ++x2) {
	  pY = rowBuf[0][y2][x1+x2];
	  pCb = rowBuf[1][y2][x1+x2] - 128;
	  pCr = rowBuf[2][y2][x1+x2] - 128;
	  pR = pY + dctCrToR * pCr;
	  if (pR < 0)
	    pR = 0;
	  else if (pR > 255)
	    pR = 255;
	  pG = pY + dctCbToG * pCb + dctCrToG * pCr;
	  if (pG < 0)
	    pG = 0;
	  else if (pG > 255)
	    pG = 255;
	  pB = pY + dctCbToB * pCb;
	  if (pB < 0)
	    pB = 0;
	  else if (pB > 255)
	    pB = 255;
	  rowBuf[0][y2][x1+x2] = (Guchar)(pR + 0.5);
	  rowBuf[1][y2][x1+x2] = (Guchar)(pG + 0.5);
	  rowBuf[2][y2][x1+x2] = (Guchar)(pB + 0.5);
	}
      }
    }
  }
  return gTrue;
}

// This IDCT algorithm is taken from:
//   Christoph Loeffler, Adriaan Ligtenberg, George S. Moschytz,
//   "Practical Fast 1-D DCT Algorithms with 11 Multiplications",
//   IEEE Intl. Conf. on Acoustics, Speech & Signal Processing, 1989,
//   988-991.
// The stage numbers mentioned in the comments refer to Figure 1 in this
// paper.
GBool DCTStream::readDataUnit(DCTHuffTable *dcHuffTable,
			      DCTHuffTable *acHuffTable,
			      Guchar quantTable[64], int *prevDC,
			      Guchar data[64]) {
  double tmp1[64];
  double v0, v1, v2, v3, v4, v5, v6, v7, t;
  int run, size, amp;
  int c;
  int i, j;

  // Huffman decode and dequantize
  size = readHuffSym(dcHuffTable);
  if (size == 9999)
    return gFalse;
  if (size > 0) {
    amp = readAmp(size);
    if (amp == 9999)
      return gFalse;
  } else {
    amp = 0;
  }
  tmp1[0] = (*prevDC += amp) * quantTable[0];
  for (i = 1; i < 64; ++i)
    tmp1[i] = 0;
  i = 1;
  while (i < 64) {
    run = 0;
    while ((c = readHuffSym(acHuffTable)) == 0xf0 && run < 0x30)
      run += 0x10;
    if (c == 9999)
      return gFalse;
    if (c == 0x00) {
      break;
    } else {
      run += (c >> 4) & 0x0f;
      size = c & 0x0f;
      amp = readAmp(size);
      if (amp == 9999)
	return gFalse;
      i += run;
      j = dctZigZag[i++];
      tmp1[j] = amp * quantTable[j];
    }
  }

  // inverse DCT on rows
  for (i = 0; i < 64; i += 8) {

    // stage 4
    v0 = dctSqrt2 * tmp1[i+0];
    v1 = dctSqrt2 * tmp1[i+4];
    v2 = tmp1[i+2];
    v3 = tmp1[i+6];
    v4 = dctSqrt1d2 * (tmp1[i+1] - tmp1[i+7]);
    v7 = dctSqrt1d2 * (tmp1[i+1] + tmp1[i+7]);
    v5 = tmp1[i+3];
    v6 = tmp1[i+5];

    /* stage 3 */
    t = 0.5 * (v0 - v1);
    v0 = 0.5 * (v0 + v1);
    v1 = t;
    t = v2 * dctSin6 + v3 * dctCos6;
    v2 = v2 * dctCos6 - v3 * dctSin6;
    v3 = t;
    t = 0.5 * (v4 - v6);
    v4 = 0.5 * (v4 + v6);
    v6 = t;
    t = 0.5 * (v7 + v5);
    v5 = 0.5 * (v7 - v5);
    v7 = t;

    /* stage 2 */
    t = 0.5 * (v0 - v3);
    v0 = 0.5 * (v0 + v3);
    v3 = t;
    t = 0.5 * (v1 - v2);
    v1 = 0.5 * (v1 + v2);
    v2 = t;
    t = v4 * dctSin3 + v7 * dctCos3;
    v4 = v4 * dctCos3 - v7 * dctSin3;
    v7 = t;
    t = v5 * dctSin1 + v6 * dctCos1;
    v5 = v5 * dctCos1 - v6 * dctSin1;
    v6 = t;

    /* stage 1 */
    tmp1[i+0] = v0 + v7;
    tmp1[i+7] = v0 - v7;
    tmp1[i+1] = v1 + v6;
    tmp1[i+6] = v1 - v6;
    tmp1[i+2] = v2 + v5;
    tmp1[i+5] = v2 - v5;
    tmp1[i+3] = v3 + v4;
    tmp1[i+4] = v3 - v4;
  }

  // inverse DCT on columns
  for (i = 0; i < 8; ++i) {

    // stage 4
    v0 = dctSqrt2 * tmp1[0*8+i];
    v1 = dctSqrt2 * tmp1[4*8+i];
    v2 = tmp1[2*8+i];
    v3 = tmp1[6*8+i];
    v4 = dctSqrt1d2 * (tmp1[1*8+i] - tmp1[7*8+i]);
    v7 = dctSqrt1d2 * (tmp1[1*8+i] + tmp1[7*8+i]);
    v5 = tmp1[3*8+i];
    v6 = tmp1[5*8+i];

    /* stage 3 */
    t = 0.5 * (v0 - v1);
    v0 = 0.5 * (v0 + v1);
    v1 = t;
    t = v2 * dctSin6 + v3 * dctCos6;
    v2 = v2 * dctCos6 - v3 * dctSin6;
    v3 = t;
    t = 0.5 * (v4 - v6);
    v4 = 0.5 * (v4 + v6);
    v6 = t;
    t = 0.5 * (v7 + v5);
    v5 = 0.5 * (v7 - v5);
    v7 = t;

    /* stage 2 */
    t = 0.5 * (v0 - v3);
    v0 = 0.5 * (v0 + v3);
    v3 = t;
    t = 0.5 * (v1 - v2);
    v1 = 0.5 * (v1 + v2);
    v2 = t;
    t = v4 * dctSin3 + v7 * dctCos3;
    v4 = v4 * dctCos3 - v7 * dctSin3;
    v7 = t;
    t = v5 * dctSin1 + v6 * dctCos1;
    v5 = v5 * dctCos1 - v6 * dctSin1;
    v6 = t;

    /* stage 1 */
    tmp1[0*8+i] = v0 + v7;
    tmp1[7*8+i] = v0 - v7;
    tmp1[1*8+i] = v1 + v6;
    tmp1[6*8+i] = v1 - v6;
    tmp1[2*8+i] = v2 + v5;
    tmp1[5*8+i] = v2 - v5;
    tmp1[3*8+i] = v3 + v4;
    tmp1[4*8+i] = v3 - v4;
  }

  // convert to 8-bit integers
  for (i = 0; i < 64; ++i) {
    if (tmp1[i] > 127)
      data[i] = 255;
    else if (tmp1[i] < -128)
      data[i] = 0;
    else
      data[i] = (Guchar)(tmp1[i] + 128.5);
  }

  return gTrue;
}

int DCTStream::readHuffSym(DCTHuffTable *table) {
  Gushort code;
  int bit;
  int codeBits;

  code = 0;
  codeBits = 0;
  do {
    // add a bit to the code
    if ((bit = readBit()) == EOF)
      return 9999;
    code = (code << 1) + bit;
    ++codeBits;

    // look up code
    if (code - table->firstCode[codeBits] < table->numCodes[codeBits]) {
      code -= table->firstCode[codeBits];
      return table->sym[table->firstSym[codeBits] + code];
    }
  } while (codeBits < 16);

  error(getPos(), "Bad Huffman code in DCT stream");
  return 9999;
}

int DCTStream::readAmp(int size) {
  int amp, bit;
  int bits;

  amp = 0;
  for (bits = 0; bits < size; ++bits) {
    if ((bit = readBit()) == EOF)
      return 9999;
    amp = (amp << 1) + bit;
  }
  if (amp < (1 << (size - 1)))
    amp -= (1 << size) - 1;
  return amp;
}

int DCTStream::readBit() {
  int bit;
  int c, c2;

  if (inputBits == 0) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    if (c == 0xff) {
      do {
	c2 = str->getChar();
      } while (c2 == 0xff);
      if (c2 != 0x00) {
	error(getPos(), "Bad DCT data: missing 00 after ff");
	return EOF;
      }
    }
    inputBuf = c;
    inputBits = 8;
  }
  bit = (inputBuf >> (inputBits - 1)) & 1;
  --inputBits;
  return bit;
}

GBool DCTStream::readHeader() {
  GBool doScan;
  int minHSample, minVSample;
  int bufWidth;
  int c = 0;
  int i, j;

  width = height = 0;
  numComps = 0;
  numQuantTables = 0;
  numDCHuffTables = 0;
  numACHuffTables = 0;
  colorXform = 0;
  restartInterval = 0;

  // read headers
  doScan = gFalse;
  while (!doScan) {
    c = readMarker();
    switch (c) {
    case 0xc0:			// SOF0
      if (!readFrameInfo())
	return gFalse;
      break;
    case 0xc4:			// DHT
      if (!readHuffmanTables())
	return gFalse;
      break;
    case 0xd8:			// SOI
      break;
    case 0xda:			// SOS
      if (!readScanInfo())
	return gFalse;
      doScan = gTrue;
      break;
    case 0xdb:			// DQT
      if (!readQuantTables())
	return gFalse;
      break;
    case 0xdd:			// DRI
      if (!readRestartInterval())
	return gFalse;
      break;
    case 0xee:			// APP14
      if (!readAdobeMarker())
	return gFalse;
      break;
    case EOF:
      error(getPos(), "Bad DCT header");
      return gFalse;
    default:
      error(getPos(), "Unknown DCT marker <%02x>", c);
      return gFalse;
    }
  }

  // compute MCU size
  mcuWidth = minHSample = compInfo[0].hSample;
  mcuHeight = minVSample = compInfo[0].vSample;
  for (i = 1; i < numComps; ++i) {
    if (compInfo[i].hSample < minHSample)
      minHSample = compInfo[i].hSample;
    if (compInfo[i].vSample < minVSample)
      minVSample = compInfo[i].vSample;
    if (compInfo[i].hSample > mcuWidth)
      mcuWidth = compInfo[i].hSample;
    if (compInfo[i].vSample > mcuHeight)
      mcuHeight = compInfo[i].vSample;
  }
  for (i = 0; i < numComps; ++i) {
    compInfo[i].hSample /= minHSample;
    compInfo[i].vSample /= minVSample;
  }
  mcuWidth = (mcuWidth / minHSample) * 8;
  mcuHeight = (mcuHeight / minVSample) * 8;

  // allocate buffers
  bufWidth = ((width + mcuWidth - 1) / mcuWidth) * mcuWidth;
  for (i = 0; i < numComps; ++i)
    for (j = 0; j < mcuHeight; ++j)
      rowBuf[i][j] = (Guchar *)gmalloc(bufWidth * sizeof(Guchar));

  // initialize counters
  comp = 0;
  x = 0;
  y = 0;
  dy = mcuHeight;

  return gTrue;
}

GBool DCTStream::readFrameInfo() {
  int length;
  int prec;
  int i;
  int c;

  length = read16() - 2;
  prec = str->getChar();
  height = read16();
  width = read16();
  numComps = str->getChar();
  length -= 6;
  if (prec != 8) {
    error(getPos(), "Bad DCT precision %d", prec);
    return gFalse;
  }
  for (i = 0; i < numComps; ++i) {
    compInfo[i].id = str->getChar();
    compInfo[i].inScan = gFalse;
    c = str->getChar();
    compInfo[i].hSample = (c >> 4) & 0x0f;
    compInfo[i].vSample = c & 0x0f;
    compInfo[i].quantTable = str->getChar();
    compInfo[i].dcHuffTable = 0;
    compInfo[i].acHuffTable = 0;
  }
  return gTrue;
}

GBool DCTStream::readScanInfo() {
  int length;
  int scanComps, id, c;
  int i, j;

  length = read16() - 2;
  scanComps = str->getChar();
  --length;
  if (length != 2 * scanComps + 3) {
    error(getPos(), "Bad DCT scan info block");
    return gFalse;
  }
  for (i = 0; i < scanComps; ++i) {
    id = str->getChar();
    for (j = 0; j < numComps; ++j) {
      if (id == compInfo[j].id)
	break;
    }
    if (j == numComps) {
      error(getPos(), "Bad DCT component ID in scan info block");
      return gFalse;
    }
    compInfo[j].inScan = gTrue;
    c = str->getChar();
    compInfo[j].dcHuffTable = (c >> 4) & 0x0f;
    compInfo[j].acHuffTable = c & 0x0f;
  }
  str->getChar();
  str->getChar();
  str->getChar();
  return gTrue;
}

GBool DCTStream::readQuantTables() {
  int length;
  int i;
  int index;

  length = read16() - 2;
  while (length > 0) {
    index = str->getChar();
    if ((index & 0xf0) || index >= 4) {
      error(getPos(), "Bad DCT quantization table");
      return gFalse;
    }
    if (index == numQuantTables)
      numQuantTables = index + 1;
    for (i = 0; i < 64; ++i)
      quantTables[index][dctZigZag[i]] = str->getChar();
    length -= 65;
  }
  return gTrue;
}

GBool DCTStream::readHuffmanTables() {
  DCTHuffTable *tbl;
  int length;
  int index;
  Gushort code;
  Guchar sym;
  int i, j;
  int c;

  length = read16() - 2;
  while (length > 0) {
    index = str->getChar();
    --length;
    if ((index & 0x0f) >= 4) {
      error(getPos(), "Bad DCT Huffman table");
      return gFalse;
    }
    if (index & 0x10) {
      index &= 0x0f;
      if (index >= numACHuffTables)
	numACHuffTables = index+1;
      tbl = &acHuffTables[index];
    } else {
      if (index >= numDCHuffTables)
	numDCHuffTables = index+1;
      tbl = &dcHuffTables[index];
    }
    sym = 0;
    code = 0;
    for (i = 1; i <= 16; ++i) {
      c = str->getChar();
      tbl->firstSym[i] = sym;
      tbl->firstCode[i] = code;
      tbl->numCodes[i] = c;
      sym += c;
      code = (code + c) << 1;
    }
    length -= 16;
    j = 0;
    for (i = 0; i < sym; ++i)
      tbl->sym[i] = str->getChar();
    length -= sym;
  }
  return gTrue;
}

GBool DCTStream::readRestartInterval() {
  int length;

  length = read16();
  if (length != 4) {
    error(getPos(), "Bad DCT restart interval");
    return gFalse;
  }
  restartInterval = read16();
  return gTrue;
}

GBool DCTStream::readAdobeMarker() {
  int length, i;
  char buf[12];
  int c;

  length = read16();
  if (length != 14)
    goto err;
  for (i = 0; i < 12; ++i) {
    if ((c = str->getChar()) == EOF)
      goto err;
    buf[i] = c;
  }
  if (strncmp(buf, "Adobe", 5))
    goto err;
  colorXform = buf[11];
  return gTrue;

 err:
  error(getPos(), "Bad DCT Adobe APP14 marker");
  return gFalse;
}

GBool DCTStream::readTrailer() {
  int c;

  c = readMarker();
  if (c != 0xd9) {		// EOI
    error(getPos(), "Bad DCT trailer");
    return gFalse;
  }
  return gTrue;
}

int DCTStream::readMarker() {
  int c;

  do {
    do {
      c = str->getChar();
    } while (c != 0xff);
    do {
      c = str->getChar();
    } while (c == 0xff);
  } while (c == 0x00);
  return c;
}

int DCTStream::read16() {
  int c1, c2;

  if ((c1 = str->getChar()) == EOF)
    return EOF;
  if ((c2 = str->getChar()) == EOF)
    return EOF;
  return (c1 << 8) + c2;
}

GString *DCTStream::getPSFilter(char *indent) {
  GString *s;

  s = str->getPSFilter(indent);
  s->append(indent)->append("<< >> /DCTDecode filter\n");
  return s;
}

GBool DCTStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}
