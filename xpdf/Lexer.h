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

#define maxTokenLen 255

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

  // Skip to the next line in the input stream.
  void skipToNextLine();

  // Get stream.
  Stream *getStream() { return str; }

  // Get current position in file.
  int getPos() { return str->getPos(); }

private:

  Stream *str;			// input stream
  int buf;			// next character
  GBool cr, lf;			// used for filtering CR/LF
  GBool freeStream;		// should Lexer free the Stream?

  int getChar();
};

#endif
