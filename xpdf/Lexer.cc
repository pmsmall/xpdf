//========================================================================
//
// Lexer.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include "Lexer.h"
#include "Error.h"

//------------------------------------------------------------------------
// Lexer
//------------------------------------------------------------------------

Lexer::Lexer(Stream *str1, Boolean freeStream1) {
  str = str1;
  buf = EOF;
  freeStream = freeStream1;
  str->reset();
}

Lexer::~Lexer() {
  if (freeStream)
    delete str;
}

Object *Lexer::getObj(Object *obj) {
  char token[maxTokenLen+1];
  char *p;
  int n;
  Boolean comment;
  Boolean real;
  int x;
  Boolean next;

  // skip whitespace and comments
  if (buf == EOF)
    buf = getChar();
  comment = false;
  while (1) {
    if (!isspace(buf)) {
      if (!comment)
	break;
      if (buf == '%')
	comment = true;
      else if (buf == '\n')
	comment = false;
      else if (buf == EOF)
	break;
    }
    buf = getChar();
  }

  // check for end of stream
  if (buf == EOF)
    return obj->initEOF();

  // start reading token
  p = token;
  n = 0;

  // number
  if ((buf >= '0' && buf <= '9') || buf == '-' || buf == '.') {
    real = false;
    do {
      if (n < maxTokenLen)
	*p++ = buf;
      ++n;
      if (buf == '.')
	real = true;
      buf = getChar();
    } while ((buf >= '0' && buf <= '9') || (!real && buf == '.'));
    *p = '\0';
    if (real)
      obj->initReal(atof(token));
    else
      obj->initInt(atoi(token));

  // string
  } else if (buf == '(') {
    buf = getChar();
    while (buf != ')' && buf != EOF) {
      next = true;
      if (buf == '\\') {
	buf = getChar();
	switch (buf) {
	case 'n':
	  if (n < maxTokenLen)
	    *p++ = '\n';
	  ++n;
	  break;
	case 'r':
	  if (n < maxTokenLen)
	    *p++ = '\r';
	  ++n;
	  break;
	case 't':
	  if (n < maxTokenLen)
	    *p++ = '\t';
	  ++n;
	  break;
	case 'b':
	  if (n < maxTokenLen)
	    *p++ = '\b';
	  ++n;
	  break;
	case 'f':
	  if (n < maxTokenLen)
	    *p++ = '\f';
	  ++n;
	  break;
	case '\\':
	  if (n < maxTokenLen)
	    *p++ = '\\';
	  ++n;
	  break;
	case '(':
	  if (n < maxTokenLen)
	    *p++ = '(';
	  ++n;
          break;
	case ')':
	  if (n < maxTokenLen)
	    *p++ = ')';
	  ++n;
	  break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	  x = buf - '0';
	  buf = getChar();
	  if (buf >= '0' && buf <= '7') {
	    x = (x << 3) + (buf - '0');
	    buf = getChar();
	    if (buf >= '0' && buf <= '7')
	      x = (x << 3) + (buf - '0');
	    else
	      next = false;
	  } else
	    next = false;
	  if (n < maxTokenLen)
	    *p++ = (char)x;
	  ++n;
	  break;
	case '\n':
	  break;
	default:
	  if (n < maxTokenLen)
	    *p++ = buf;
	  ++n;
	  break;
	}
      } else {
	if (n < maxTokenLen)
	  *p++ = buf;
	++n;
      }
      if (next)
	buf = getChar();
    }
    *p = '\0';
    if (buf == EOF)
      error(getPos(), "End of file inside string");
    else
      buf = EOF;  // kill the closing paren
    obj->initString(token);

  // name
  } else if (buf == '/') {
    buf = getChar();
    while (!isspace(buf) && buf!= '/' && buf != '%' && buf != '(' &&
	   buf != ')' && buf != '<' && buf != '>' && buf != '[' &&
	   buf != ']' && buf != '{' && buf != '}' && buf != EOF) {
      if (n < maxTokenLen)
	*p++ = buf;
      ++n;
      buf = getChar();
    }
    *p = '\0';
    obj->initName(token);

  // array punctuation
  } else if (buf == '[' || buf == ']') {
    token[0] = buf;
    token[1] = '\0';
    buf = EOF;
    obj->initCmd(token);

  // hex string or dict punctuation
  } else if (buf == '<') {
    buf = getChar();

    // dict punctuation
    if (buf == '<') {
      token[0] = token[1] = '<';
      token[2] = '\0';
      buf = EOF;
      obj->initCmd(token);

    // hex string
    } else {
      while (1) {
	while (isspace(buf))
	  buf = getChar();
	if (buf == '>') {
	  buf = EOF;
	  break;
	}
	if (buf == EOF) {
	  error(getPos(), "End of file inside hex string");
	  break;
	}
	if (buf >= '0' && buf <= '9') {
	  x = (buf - '0') << 4;
	} else if (buf >= 'A' && buf <= 'F') {
	  x = (buf - 'A' + 10) << 4;
	} else if (buf >= 'a' && buf <= 'f') {
	  x = (buf - 'a' + 10) << 4;
	} else {
	  error(getPos(), "Illegal character <%02x> in hex string", buf);
	  x = 0;
	}
	do {
	  buf = getChar();
	} while (isspace(buf));
	if (buf == EOF) {
	  error(getPos(), "End of file inside hex string");
	  break;
	}
	if (buf >= '0' && buf <= '9') {
	  x += buf - '0';
	} else if (buf >= 'A' && buf <= 'F') {
	  x += buf - 'A' + 10;
	} else if (buf >= 'a' && buf <= 'f') {
	  x += buf - 'a' + 10;
	} else if (buf != '>') {
	  error(getPos(), "Illegal character <%02x> in hex string", buf);
	}
	if (n < maxTokenLen)
	  *p++ = (char)x;
	++n;
	if (buf == '>') {
	  buf = EOF;
	  break;
	}
	buf = getChar();
      }
      obj->initString(token);
    }

  // dict punctuation
  } else if (buf == '>') {
    buf = getChar();
    if (buf == '>') {
      token[0] = token[1] = '>';
      token[2] = '\0';
      buf = EOF;
      obj->initCmd(token);
    } else {
      error(getPos(), "Illegal character '>'");
      obj->initError();
    }

  // command
  } else if (buf != ')' && buf != '{' && buf != '}') {
    do {
      if (n < maxTokenLen)
	*p++ = buf;
      ++n;
      buf = getChar();
    } while (!isspace(buf) && buf != '%' && buf != '(' && buf != ')' &&
	     buf != '<' && buf != '>' && buf != '[' && buf != ']' &&
	     buf != '{' && buf != '}' && buf != EOF);
    *p = '\0';
    if (!strcmp(token, "true"))
      obj->initBool(true);
    else if (!strcmp(token, "false"))
      obj->initBool(false);
    else
      obj->initCmd(token);

  // error
  } else {
    error(getPos(), "Illegal character '%c'", buf);
    buf = EOF;  // eat the character to avoid an infinite loop
    obj->initError();
  }

  return obj;
}

void Lexer::skipToNextLine() {
  if (buf == EOF)
    buf = getChar();
  while (buf != '\n' && buf != EOF)
    buf = getChar();
  getChar();  // skip over end-of-line chars
  buf = EOF;
}

int Lexer::getChar() {
  int c;

 start:
  c = str->getChar();
  if (c != '\n' && c != '\r') {
    lf = cr = false;
  } else {
    if (c == '\n') {
      lf = true;
    } else {
      cr = true;
      c = '\n';
    }
    if (lf && cr) {
      lf = cr = false;
      goto start;
    }
  }
  return c;
}
