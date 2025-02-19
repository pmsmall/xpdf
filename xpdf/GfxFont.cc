//========================================================================
//
// GfxFont.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <String.h>
#include <mem.h>
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
  int firstChar, lastChar;
  Object obj, obj2;
  int i, j;

  // get font tag
  tag = new String(tag1);

  // get base font name
  name = NULL;
  fontDict->lookup("BaseFont", &obj);
  if (obj.isName())
    name = new String(obj.getName());
  obj.free();

  // get encoding
  fontDict->lookup("Encoding", &obj);
  if (obj.isName("MacRomanEncoding")) {
    memcpy(isoMap, macRomanISOMap, sizeof(isoMap));
    memcpy(pdfMap, macRomanPDFMap, sizeof(pdfMap));
  } else if (obj.isName("WinAnsiEncoding")) {
    memcpy(isoMap, winAnsiISOMap, sizeof(isoMap));
    memcpy(pdfMap, winAnsiPDFMap, sizeof(pdfMap));
  } else if (obj.isDict()) {
    getEncoding(obj.getDict());
  } else {
    if (obj.isName())
      error(0, "Unknown font encoding '%s'", obj.getName());
    else if (!obj.isNull())
      error(0, "Font encoding is wrong type (%s)", obj.getTypeName());
    memcpy(isoMap, standardISOMap, sizeof(isoMap));
    memcpy(pdfMap, standardPDFMap, sizeof(pdfMap));
  }
  obj.free();
  for (i = 0; i < 256; ++i)
    revISOMap[isoMap[i]] = i;

  // get character widths
  for (i = 0; i < numBuiltinFonts; ++i) {
    if (name && !strcmp(builtinFonts[i].name, name->getCString()))
      break;
  }
  if (i < numBuiltinFonts) {
    for (j = 0; j < 256; ++j)
      widths[j] = (double)builtinFonts[i].widths[pdfMap[j]] / 1000.0;
  } else {
    fontDict->lookup("FirstChar", &obj);
    firstChar = obj.isInt() ? obj.getInt() : 0;
    obj.free();
    fontDict->lookup("LastChar", &obj);
    lastChar = obj.isInt() ? obj.getInt() : 255;
    obj.free();
    for (i = 0; i < 256; ++i)
      widths[i] = 0;
    fontDict->lookup("Widths", &obj);
    if (obj.isArray()) {
      for (i = firstChar; i < lastChar; ++i) {
	obj.arrayGet(i - firstChar, &obj2);
	if (obj2.isNum())
	  widths[i] = obj2.getNum() / 1000;
	obj2.free();
      }
    } else {
      error(0, "No character widths resource for non-builtin font");
      for (i = 0; i < 255; ++i)
	widths[i] = 0.3;
    }
    obj.free();
  }

  // get info from font descriptor
  fontDict->lookup("FontDescriptor", &obj);
  if (obj.isDict()) {
    obj.dictLookup("Flags", &obj2);
    if (obj2.isInt())
      flags = obj2.getInt();
    obj2.free();
  }
  obj.free();
}

void GfxFont::getEncoding(Dict *dict) {
  Object obj1, obj2;
  int i, j;

  // base encoding
  dict->lookup("BaseEncoding", &obj1);
  if (obj1.isName("MacRomanEncoding")) {
    memcpy(isoMap, macRomanISOMap, sizeof(isoMap));
    memcpy(pdfMap, macRomanPDFMap, sizeof(pdfMap));
  } else if (obj1.isName("WinAnsiEncoding")) {
    memcpy(isoMap, winAnsiISOMap, sizeof(isoMap));
    memcpy(pdfMap, winAnsiPDFMap, sizeof(pdfMap));
  } else {
    memcpy(isoMap, standardISOMap, sizeof(isoMap));
    memcpy(pdfMap, standardPDFMap, sizeof(pdfMap));
  }
  obj1.free();

  // differences array
  dict->lookup("Differences", &obj1);
  if (obj1.isArray()) {
    j = 0;
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      obj1.arrayGet(i, &obj2);
      if (obj2.isInt()) {
	j = obj2.getInt();
      } else if (obj2.isName()) {
	if (j < 256)
	  findNamedChar(obj2.getName(), &isoMap[j], &pdfMap[j]);
	++j;
      } else {
	error(0, "Wrong type in font encoding resource differences (%s)",
	      obj2.getTypeName());
      }
      obj2.free();
    }
  }
  obj1.free();
}

void GfxFont::findNamedChar(char *name, ushort *isoCode, ushort *pdfCode) {
  int a, b, m, cmp;

  // check for a name of the form 'aN'
  if (name[0] == 'a' && name[1] >= '0' && name[1] <= '9') {
    *isoCode = *pdfCode = atoi(name+1);
    return;
  }

  // invariant: namedEncoding[a] < name < namedEncoding[b]
  a = -1;
  b = numNamedChars;
  while (b - a > 1) {
    m = (a + b) / 2;
    cmp = strcmp(namedEncoding[m].name, name);
    if (cmp < 0)
      a = m;
    else if (cmp > 0)
      b = m;
    else
      a = b = m;
  }
  if (cmp == 0) {
    *isoCode = namedEncoding[m].code;
    *pdfCode = m;
  } else {
    *isoCode = 0;
    *pdfCode = 0;
  }
}

GfxFont::~GfxFont() {
  delete tag;
  if (name)
    delete name;
}

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

GfxFontDict::GfxFontDict(Dict *fontDict) {
  int i;
  Object obj;

  numFonts = fontDict->getLength();
  fonts = (GfxFont **)smalloc(numFonts * sizeof(GfxFont *));
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
  sfree(fonts);
}

GfxFont *GfxFontDict::lookup(char *tag) {
  int i;

  for (i = 0; i < numFonts; ++i) {
    if (fonts[i]->matches(tag))
      return fonts[i];
  }
  return NULL;
}
