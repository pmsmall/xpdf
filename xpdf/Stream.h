//========================================================================
//
// Stream.h
//
//========================================================================

#ifndef STREAM_H
#define STREAM_H

#pragma interface

#include <stdio.h>
#include <stypes.h>

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

  // Get the base file of this stream.
  virtual FILE *getFile() = 0;

  // Get the dictionary associated with this stream.
  virtual Dict *getDict() = 0;

  // Check for a PDF header on this stream.
  Boolean checkHeader();

private:

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
  virtual FILE *getFile() { return f; }
  virtual Dict *getDict();

private:

  FILE *f;
  int start;
  int length;
  int pos;
  int savePos;
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
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  Boolean eof;
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
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  ulong c[5];
  ulong b[4];
  int index, n;
  Boolean eof;
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
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int predictor;		// parameters
  int columns;
  int colors;
  int bits;
  int early;
  Boolean eof;			// true if at eof
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  struct {			// decoding table
    int length;
    int head;
    uchar tail;
  } table[4096];
  int nextCode;			// next code to be used
  int nextBits;			// number of bits in next code word
  int prevCode;			// previous code used in stream
  int newChar;			// next char to be added to table
  uchar seqBuf[4096];		// buffer for current sequence
  int seqLength;		// length of current sequence
  int seqIndex;			// index into current sequence
  Boolean first;		// first code after a table clear

  void clearTable();
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

  CCITTFaxStream(Stream *str1, int encoding1, Boolean byteAlign1,
		 int columns1, int rows1, Boolean black1);
  virtual ~CCITTFaxStream();
  virtual void reset();
  virtual int getChar();
  virtual int getPos() { return str->getPos(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int encoding;			// 'K' parameter
  Boolean byteAlign;		// 'EncodedByteAlign' parameter
  int columns;			// 'Columns' parameter
  int rows;			// 'Rows' parameter
  Boolean black;		// 'BlackIs1' parameter
  Boolean eof;			// true if at eof
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

#if 0
//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

class DCTStream: public Stream {
};
#endif

#endif
