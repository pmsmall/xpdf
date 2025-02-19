//========================================================================
//
// Lexer.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "Lexer.h"
#include "Error.h"

//------------------------------------------------------------------------
// Lexer
//------------------------------------------------------------------------

Lexer::Lexer(Stream *str1, GBool freeStream1) {
  str = str1;
  str->reset();
  freeStream = freeStream1;
}

Lexer::~Lexer() {
  if (freeStream)
    delete str;
}

Object *Lexer::getObj(Object *obj) {
  char *p;
  int c, c2;
  GBool comment, neg, done;
  int xi;
  double xf, scale;
  GString *s;
  int n, m;

  // skip whitespace and comments
  comment = gFalse;
  while (1) {
    if ((c = str->getChar()) == EOF)
      return obj->initEOF();
    if (comment) {
      if (c == '\r' || c == '\n')
	comment = gFalse;
    } else if (c == '%') {
      comment = gTrue;
    } else if (!isspace(c)) {
      break;
    }
  }

  // start reading token
  switch (c) {

  // number
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  case '-': case '.':
    neg = gFalse;
    xi = 0;
    if (c == '-') {
      neg = gTrue;
    } else if (c == '.') {
      goto doReal;
    } else {
      xi = c - '0';
    }
    while (1) {
      c = str->lookChar();
      if (isdigit(c)) {
	str->getChar();
	xi = xi * 10 + (c - '0');
      } else if (c == '.') {
	str->getChar();
	goto doReal;
      } else {
	break;
      }
    }
    if (neg)
      xi = -xi;
    obj->initInt(xi);
    break;
  doReal:
    xf = xi;
    scale = 0.1;
    while (1) {
      c = str->lookChar();
      if (!isdigit(c))
	break;
      str->getChar();
      xf = xf + scale * (c - '0');
      scale *= 0.1;
    }
    if (neg)
      xf = -xf;
    obj->initReal(xf);
    break;

  // string
  case '(':
    p = tokBuf;
    n = 0;
    done = gFalse;
    s = NULL;
    do {
      c2 = EOF;
      switch (c = str->getChar()) {

      case EOF:
      case '\r':
      case '\n':
	error(getPos(), "Unterminated string");
	done = gTrue;
	break;

      case ')':
	done = gTrue;
	break;

      case '\\':
	switch (c = str->getChar()) {
	case 'n':
	  c2 = '\n';
	  break;
	case 'r':
	  c2 = '\r';
	  break;
	case 't':
	  c2 = '\t';
	  break;
	case 'b':
	  c2 = '\b';
	  break;
	case 'f':
	  c2 = '\f';
	  break;
	case '\\':
	case '(':
	case ')':
	  c2 = c;
	  break;
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	  c2 = c - '0';
	  c = str->lookChar();
	  if (c >= '0' && c <= '7') {
	    str->getChar();
	    c2 = (c2 << 3) + (c - '0');
	    c = str->lookChar();
	    if (c >= '0' && c <= '7') {
	      str->getChar();
	      c2 = (c2 << 3) + (c - '0');
	    }
	  }
	  break;
	case '\r':
	  c = str->lookChar();
	  if (c == '\n')
	    str->getChar();
	  break;
	case '\n':
	  break;
	case EOF:
	  error(getPos(), "Unterminated string");
	  done = gTrue;
	  break;
	default:
	  c2 = c;
	  break;
	}
	break;

      default:
	c2 = c;
	break;
      }

      if (c2 != EOF) {
	if (n == tokBufSize) {
	  if (!s)
	    s = new GString(tokBuf, tokBufSize);
	  else
	    s->append(tokBuf, tokBufSize);
	  p = tokBuf;
	  n = 0;
	}
	*p++ = (char)c2;
	++n;
      }
    } while (!done);
    if (!s)
      s = new GString(tokBuf, n);
    else
      s->append(tokBuf, n);
    obj->initString(s);
    break;

  // name
  case '/':
    p = tokBuf;
    n = 0;
    while ((c = str->lookChar()) != EOF && !isspace(c) &&
	   c != '/' && c != '%' && c != '(' && c != ')' &&
	   c != '<' && c != '>' && c != '[' && c != ']' &&
	   c != '{' && c != '}') {
      str->getChar();
      if (++n == tokBufSize) {
	error(getPos(), "Name token too long");
	break;
      }
      *p++ = c;
    }
    *p = '\0';
    obj->initName(tokBuf);
    break;

  // array punctuation
  case '[':
  case ']':
    tokBuf[0] = c;
    tokBuf[1] = '\0';
    obj->initCmd(tokBuf);
    break;

  // hex string or dict punctuation
  case '<':
    c = str->lookChar();

    // dict punctuation
    if (c == '<') {
      str->getChar();
      tokBuf[0] = tokBuf[1] = '<';
      tokBuf[2] = '\0';
      obj->initCmd(tokBuf);

    // hex string
    } else {
      p = tokBuf;
      m = n = 0;
      c2 = 0;
      s = NULL;
      while (1) {
	c = str->getChar();
	if (c == '>') {
	  break;
	} else if (c == EOF) {
	  error(getPos(), "Unterminated hex string");
	  break;
	} else if (!isspace(c)) {
	  c2 = c2 << 4;
	  if (c >= '0' && c <= '9')
	    c2 += (c - '0');
	  else if (c >= 'A' && c <= 'F')
	    c2 += (c - 'A' + 10);
	  else if (c >= 'a' && c <= 'f')
	    c2 += (c - 'a' + 10);
	  else
	    error(getPos(), "Illegal character <%02x> in hex string", c);
	  if (++m == 2) {
	    if (n == tokBufSize) {
	      if (!s)
		s = new GString(tokBuf, tokBufSize);
	      else
		s->append(tokBuf, tokBufSize);
	      p = tokBuf;
	      n = 0;
	    }
	    *p++ = (char)c2;
	    ++n;
	    c2 = 0;
	    m = 0;
	  }
	}
      }
      if (!s)
	s = new GString(tokBuf, n);
      else
	s->append(tokBuf, n);
      if (m == 1)
	s->append((char)(c2 << 4));
      obj->initString(s);
    }
    break;

  // dict punctuation
  case '>':
    c = str->lookChar();
    if (c == '>') {
      str->getChar();
      tokBuf[0] = tokBuf[1] = '>';
      tokBuf[2] = '\0';
      obj->initCmd(tokBuf);
    } else {
      error(getPos(), "Illegal character '>'");
      obj->initError();
    }
    break;

  // error
  case ')':
  case '{':
  case '}':
    error(getPos(), "Illegal character '%c'", c);
    obj->initError();
    break;

  // command
  default:
    p = tokBuf;
    *p++ = c;
    n = 1;
    while ((c = str->lookChar()) != EOF && !isspace(c) &&
	   c != '%' && c != '(' && c != ')' &&
	   c != '<' && c != '>' && c != '[' && c != ']' &&
	   c != '{' && c != '}') {
      str->getChar();
      if (++n == tokBufSize) {
	error(getPos(), "Command token too long");
	break;
      }
      *p++ = c;
    }
    *p = '\0';
    if (!strcmp(tokBuf, "true"))
      obj->initBool(gTrue);
    else if (!strcmp(tokBuf, "false"))
      obj->initBool(gFalse);
    else if (!strcmp(tokBuf, "null"))
      obj->initNull();
    else
      obj->initCmd(tokBuf);
    break;
  }

  return obj;
}

void Lexer::skipToNextLine() {
  int c;

  while (1) {
    c = str->getChar();
    if (c == EOF || c == '\n')
      return;
    if (c == '\r') {
      if ((c = str->lookChar()) == '\n')
	str->getChar();
      return;
    }
  }
}
