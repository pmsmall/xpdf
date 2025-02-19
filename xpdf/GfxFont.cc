//========================================================================
//
// GfxFont.cc
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
#include <GString.h>
#include <gmem.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Error.h"
#include "GfxFont.h"

#include "FontInfo.h"

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

GfxFont::GfxFont(char *tag1, Dict *fontDict) {
  BuiltinFont *builtinFont;
  Object obj1, obj2;
  int i;

  // get font tag
  tag = new GString(tag1);

  // get base font name
  name = NULL;
  fontDict->lookup("BaseFont", &obj1);
  if (obj1.isName())
    name = new GString(obj1.getName());
  obj1.free();

  // is it a built-in font?
  builtinFont = NULL;
  if (name) {
    for (i = 0; i < numBuiltinFonts; ++i) {
      if (!strcmp(builtinFonts[i].name, name->getCString())) {
	builtinFont = &builtinFonts[i];
	break;
      }
    }
  }

  // get encoding and character widths
  if (builtinFont) {
    makeEncoding(fontDict, builtinFont->encoding);
    makeWidths(fontDict, builtinFont->encoding, builtinFont->encodingSize,
	       builtinFont->widths);
  } else {
    makeEncoding(fontDict, NULL);
    makeWidths(fontDict, NULL, 0, NULL);
  }

  // get info from font descriptor
  flags = 0;
  fontDict->lookup("FontDescriptor", &obj1);
  if (obj1.isDict()) {
    obj1.dictLookup("Flags", &obj2);
    if (obj2.isInt())
      flags = obj2.getInt();
    obj2.free();
  }
  obj1.free();
}

int GfxFont::lookupCharName(char *name, char **enc, int encSize, int hint) {
  int code;

  // check hinted code
  if (enc[hint] && !strcmp(name, enc[hint]))
    return hint;

  // search for it
  for (code = 0; code < encSize; ++code) {
    if (enc[code] && !strcmp(name, enc[code]))
      return code;
  }
  return -1;
}

double GfxFont::getWidth(GString *s) {
  double w;
  int i;

  w = 0;
  for (i = 0; i < s->getLength(); ++i)
    w += widths[s->getChar(i) & 0xff];
  return w;
}

void GfxFont::makeEncoding(Dict *fontDict, char **builtinEncoding) {
  char **baseEncoding;
  Object obj1, obj2, obj3;
  int code, i;

  // start with empty encoding
  for (code = 0; code < 256; ++code)
    encoding[code] = NULL;

  // try to get encoding from font dict
  fontDict->lookup("Encoding", &obj1);

  // MacRoman or WinAnsi encoding
  if (obj1.isName("MacRomanEncoding")) {
    baseEncoding = macRomanEncoding;
  } else if (obj1.isName("WinAnsiEncoding")) {
    baseEncoding = winAnsiEncoding;

  // custom encoding
  } else if (obj1.isDict()) {
    obj1.dictLookup("BaseEncoding", &obj2);
    if (obj2.isName("MacRomanEncoding"))
      baseEncoding = macRomanEncoding;
    else if (obj2.isName("WinAnsiEncoding"))
      baseEncoding = winAnsiEncoding;
    else if (builtinEncoding)
      baseEncoding = builtinEncoding;
    else
      baseEncoding = standardEncoding;
    obj2.free();
    obj1.dictLookup("Differences", &obj2);
    if (obj2.isArray()) {
      code = 0;
      for (i = 0; i < obj2.arrayGetLength(); ++i) {
	obj2.arrayGet(i, &obj3);
	if (obj3.isInt()) {
	  code = obj3.getInt();
	} else if (obj3.isName()) {
	  if (code < 256) {
	    gfree(encoding[code]);
	    encoding[code] = copyString(obj3.getName());
	  }
	  ++code;
	} else {
	  error(0, "Wrong type in font encoding resource differences (%s)",
		obj3.getTypeName());
	}
	obj3.free();
      }
    }
    obj2.free();

  // built-in font encoding
  } else if (builtinEncoding) {
    baseEncoding = builtinEncoding;
    if (!obj1.isNull())
      error(0, "Font encoding is wrong type (%s)", obj1.getTypeName());

  // default encoding
  } else {
    if (obj1.isName())
      error(0, "Unknown font encoding '%s'", obj1.getName());
    else if (!obj1.isNull())
      error(0, "Font encoding is wrong type (%s)", obj1.getTypeName());
    baseEncoding = standardEncoding;
  }

  // free the font dict encoding
  obj1.free();

  // merge base encoding and differences
  for (code = 0; code < 256; ++code) {
    if (!encoding[code] && baseEncoding[code])
      encoding[code] = copyString(baseEncoding[code]);
  }
}

void GfxFont::makeWidths(Dict *fontDict, char **builtinEncoding,
			 int builtinEncodingSize, Gushort *builtinWidths) {
  Object obj1, obj2;
  int firstChar, lastChar;
  int code, code2;
  double scale;

  // use widths from built-in font
  if (builtinEncoding) {
    for (code = 0; code < 256; ++code) {
      code2 = lookupCharName(encoding[code], builtinEncoding,
			     builtinEncodingSize, code);
      if (code2 >= 0)
	widths[code] = (code2 >= 0) ? (builtinWidths[code2] / 1000.0) : 0.0;
    }

  // get widths from font dict
  } else {
    fontDict->lookup("FirstChar", &obj1);
    firstChar = obj1.isInt() ? obj1.getInt() : 0;
    obj1.free();
    fontDict->lookup("LastChar", &obj1);
    lastChar = obj1.isInt() ? obj1.getInt() : 255;
    obj1.free();
    for (code = 0; code < 256; ++code)
      widths[code] = 0;
    fontDict->lookup("Widths", &obj1);
    if (obj1.isArray()) {
      for (code = firstChar; code < lastChar; ++code) {
	obj1.arrayGet(code - firstChar, &obj2);
	if (obj2.isNum())
	  widths[code] = obj2.getNum() / 1000;
	obj2.free();
      }
    } else {
      error(0, "No character widths resource for non-builtin font");
      for (code = 0; code < 255; ++code)
	widths[code] = 0.3;
    }
    obj1.free();
  }
}

GfxFont::~GfxFont() {
  int i;

  delete tag;
  if (name)
    delete name;
  for (i = 0; i < 256; ++i) {
    if (encoding[i])
      gfree(encoding[i]);
  }
}

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

GfxFontDict::GfxFontDict(Dict *fontDict) {
  int i;
  Object obj;

  numFonts = fontDict->getLength();
  fonts = (GfxFont **)gmalloc(numFonts * sizeof(GfxFont *));
  for (i = 0; i < numFonts; ++i) {
    fontDict->getVal(i, &obj);
    if (obj.isDict("Font")) {
      fonts[i] = new GfxFont(fontDict->getKey(i), obj.getDict());
    } else {
      error(0, "font resource is not a dictionary");
      fonts[i] = NULL;
    }
    obj.free();
  }
}

GfxFontDict::~GfxFontDict() {
  int i;

  for (i = 0; i < numFonts; ++i)
    delete fonts[i];
  gfree(fonts);
}

GfxFont *GfxFontDict::lookup(char *tag) {
  int i;

  for (i = 0; i < numFonts; ++i) {
    if (fonts[i]->matches(tag))
      return fonts[i];
  }
  return NULL;
}
