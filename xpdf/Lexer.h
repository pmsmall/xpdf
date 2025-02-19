//========================================================================
//
// Lexer.h
//
//========================================================================

#ifndef LEXER_H
#define LEXER_H

#pragma interface

#include "Object.h"
#include "Stream.h"

#define maxTokenLen 255

//------------------------------------------------------------------------
// Lexer
//------------------------------------------------------------------------

class Lexer {
public:

  // Constructor.
  Lexer(Stream *str1, Boolean freeStream1 = true);

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
  Boolean cr, lf;		// used for filtering CR/LF
  Boolean freeStream;		// should Lexer free the Stream?

  int getChar();
  void skipSpace();
};

#endif
