//========================================================================
//
// Parser.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef PARSER_H
#define PARSER_H

#pragma interface

#include "Lexer.h"

//------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------

class Parser {
public:

  // Constructor.
  Parser(Lexer *lexer1);

  // Destructor.
  ~Parser();

  // Get the next object from the input stream.
  Object *getObj(Object *obj);

  // Get stream.
  Stream *getStream() { return lexer->getStream(); }

  // Get current position in file.
  int getPos() { return lexer->getPos(); }

private:

  Lexer *lexer;			// input stream
  Object buf1, buf2;		// next two tokens
  int inlineImg;		// set when inline image data is encountered

  Stream *makeStream(Object *dict);
  void shift();
};

#endif

