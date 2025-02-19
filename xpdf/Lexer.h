//========================================================================
//
// Lexer.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LEXER_H
#define LEXER_H

#ifdef __GNUC__
#pragma interface
#endif

#include "Object.h"
#include "Stream.h"

#define tokBufSize 128		// size of token buffer

//------------------------------------------------------------------------
// Lexer
//------------------------------------------------------------------------

class Lexer {
public:

  // Constructor.
  Lexer(Stream *str1, GBool freeStream1 = gTrue);

  // Destructor.
  ~Lexer();

  // Get the next object from the input stream.
  Object *getObj(Object *obj);

  // Skip to the beginning of the next line in the input stream.
  void skipToNextLine();

  // Skip over one character.
  void skipChar() { str->getChar(); }

  // Get stream.
  Stream *getStream() { return str; }

  // Get current position in file.
  int getPos() { return str->getPos(); }

  // Set position in file.
  void setPos(int pos) { str->setPos(pos); }

private:

  Stream *str;			// input stream
  GBool freeStream;		// should Lexer free the Stream?
  char tokBuf[tokBufSize];	// temporary token buffer
};

#endif
