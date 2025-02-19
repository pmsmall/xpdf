//========================================================================
//
// Parser.h
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

  // Get current position in file.
  int getPos() { return lexer->getPos(); }

private:

  Lexer *lexer;			// input stream
  Object buf1, buf2;		// next two tokens

  Stream *makeStream(Object *dict);
  Stream *makeFilter(char *name, Stream *str, Object *params);
  void shift();
};

#endif

