//========================================================================
//
// Stream.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef STREAM_H
#define STREAM_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <gtypes.h>

#include "Object.h"

//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

class Stream {
public:

  // Constructor.
  Stream(): ref(1) {}

  // Destructor.
  virtual ~Stream() {}

  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Reset stream to beginning.
  virtual void reset() = 0;

  // Get next char from stream.
  virtual int getChar() = 0;

  // Get current position in file.
  virtual int getPos() = 0;

  // Go to a position in the stream.
  virtual void setPos(int pos1);

  // Get name of i'th filter.
  virtual char *getFilter(int i) { return NULL; }

  // Does this stream type potentially contain non-printable chars?
  virtual GBool isBinary() { return gTrue; }

  // Get the base FileStream or SubStream of this stream.
  virtual Stream *getBaseStream() = 0;

  // Get the base file of this stream.
  virtual FILE *getFile() = 0;

  // Get the dictionary associated with this stream.
  virtual Dict *getDict() = 0;

  // Check for a PDF header on this stream.
  GBool checkHeader();

  // Add filters to this stream according to the parameters in <dict>.
  // Returns the new stream.
  Stream *addFilters(Object *dict);

private:

  Stream *makeFilter(char *name, Stream *str, Object *params);

  int ref;			// reference count
};

//------------------------------------------------------------------------
// FileStream
//------------------------------------------------------------------------

class FileStream: public Stream {
public:

  FileStream(FILE *f1, int start1, int length1, Object *dict1);
  virtual ~FileStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return pos; }
  virtual void setPos(int pos1);
  virtual Stream *getBaseStream() { return this; }
  virtual FILE *getFile() { return f; }
  virtual Dict *getDict() { return dict.getDict(); }

private:

  FILE *f;
  int start;
  int length;
  int pos;
  int savePos;
  Object dict;
};

//------------------------------------------------------------------------
// SubStream
//------------------------------------------------------------------------

class SubStream: public Stream {
public:

  SubStream(Stream *str1, Object *dict1);
  virtual ~SubStream();
  virtual void reset() {}
  virtual int getChar() { return str->getChar(); }
  virtual int getPos() { return str->getPos(); }
  virtual Stream *getBaseStream() { return this; }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return dict.getDict(); }

private:

  Stream *str;
  Object dict;
};

//------------------------------------------------------------------------
// ASCIIHexStream
//------------------------------------------------------------------------

class ASCIIHexStream: public Stream {
public:

  ASCIIHexStream(Stream *str1);
  virtual ~ASCIIHexStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual char *getFilter(int i);
  virtual GBool isBinary() { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  GBool eof;
};

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

class ASCII85Stream: public Stream {
public:

  ASCII85Stream(Stream *str1);
  virtual ~ASCII85Stream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual char *getFilter(int i);
  virtual GBool isBinary() { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  Gulong c[5];
  Gulong b[4];
  int index, n;
  GBool eof;
};

//------------------------------------------------------------------------
// LZWStream
//------------------------------------------------------------------------

class LZWStream: public Stream {
public:

  LZWStream(Stream *str1, int predictor1, int columns1, int colors1,
	    int bits1, int early1);
  virtual ~LZWStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual char *getFilter(int i);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int predictor;		// parameters
  int columns;
  int colors;
  int bits;
  int early;
  char zCmd[256];		// uncompress command
  FILE *zPipe;			// uncompress pipe
  char *zName;			// .Z file name (in zCmd)
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  int inCodeBits;		// size of input code

  void dumpFile(FILE *f);
  int getCode();
};

#if 0
//------------------------------------------------------------------------
// RunLengthStream
//------------------------------------------------------------------------

class RunLengthStream: public Stream {
};
#endif

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

struct CCITTCodeTable;

class CCITTFaxStream: public Stream {
public:

  CCITTFaxStream(Stream *str1, int encoding1, GBool byteAlign1,
		 int columns1, int rows1, GBool black1);
  virtual ~CCITTFaxStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual char *getFilter(int i);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int encoding;			// 'K' parameter
  GBool byteAlign;		// 'EncodedByteAlign' parameter
  int columns;			// 'Columns' parameter
  int rows;			// 'Rows' parameter
  GBool black;			// 'BlackIs1' parameter
  GBool eof;			// true if at eof
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  short *refLine;		// reference line changing elements
  int b1;			// index into refLine
  short *codingLine;		// coding line changing elements
  int a0;			// index into codingLine
  int outputBits;		// remaining ouput bits

  short getCode(CCITTCodeTable *table);
  int getBit();
};

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

// DCT component info
struct DCTCompInfo {
  int id;			// component ID
  GBool inScan;			// is this component in the current scan?
  int hSample, vSample;		// horiz/vert sampling resolutions
  int quantTable;		// quantization table number
  int dcHuffTable, acHuffTable;	// Huffman table numbers
  int prevDC;			// DC coefficient accumulator
};

// DCT Huffman decoding table
struct DCTHuffTable {
  Guchar firstSym[17];		// first symbol for this bit length
  Gushort firstCode[17];	// first code for this bit length
  Gushort numCodes[17];		// number of codes of this bit length
  Guchar sym[256];		// symbols
};

class DCTStream: public Stream {
public:

  DCTStream(Stream *str1);
  virtual ~DCTStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual char *getFilter(int i);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int width, height;		// image size
  int mcuWidth, mcuHeight;	// size of min coding unit, in data units
  DCTCompInfo compInfo[4];	// info for each component
  int numComps;			// number of components in image
  int colorXform;		// need YCbCr-to-RGB transform?
  int restartInterval;		// restart interval, in MCUs
  Guchar quantTables[4][64];	// quantization tables
  int numQuantTables;		// number of quantization tables
  DCTHuffTable dcHuffTables[4];	// DC Huffman tables
  DCTHuffTable acHuffTables[4];	// AC Huffman tables
  int numDCHuffTables;		// number of DC Huffman tables
  int numACHuffTables;		// number of AC Huffman tables
  Guchar *rowBuf[4][32];	// buffer for one MCU
  int comp, x, y, dy;		// current position within image/MCU
  int restartCtr;		// MCUs left until restart
  int restartMarker;		// next restart marker
  int inputBuf;			// input buffer for variable length codes
  int inputBits;		// number of valid bits in input buffer

  void restart();
  GBool readMCURow();
  GBool readDataUnit(DCTHuffTable *dcHuffTable, DCTHuffTable *acHuffTable,
		     Guchar quantTable[64], int *prevDC, Guchar data[64]);
  int readHuffSym(DCTHuffTable *table);
  int readAmp(int size);
  int readBit();
  GBool readHeader();
  GBool readFrameInfo();
  GBool readScanInfo();
  GBool readQuantTables();
  GBool readHuffmanTables();
  GBool readRestartInterval();
  GBool readAdobeMarker();
  GBool readTrailer();
  int readMarker();
  int read16();
};

#endif
