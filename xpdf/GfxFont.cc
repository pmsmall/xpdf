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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <GString.h>
#include <gmem.h>
#include <gfile.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Error.h"
#include "Params.h"
#include "GfxFont.h"

#include "FontInfo.h"

//------------------------------------------------------------------------

static Gushort *defCharWidths[12] = {
  courierWidths,
  courierObliqueWidths,
  courierBoldWidths,
  courierBoldObliqueWidths,
  helveticaWidths,
  helveticaObliqueWidths,
  helveticaBoldWidths,
  helveticaBoldObliqueWidths,
  timesRomanWidths,
  timesItalicWidths,
  timesBoldWidths,
  timesBoldItalicWidths
};

//------------------------------------------------------------------------
// GfxFontEncoding
//------------------------------------------------------------------------

inline int GfxFontEncoding::hash(char *name) {
  int h;

  h = name[0];
  if (name[1])
    h = h * 61 + name[1];
  return h % gfxFontEncHashSize;
}

GfxFontEncoding::GfxFontEncoding() {
  int i;

  encoding = (char **)gmalloc(256 * sizeof(char *));
  freeEnc = gTrue;
  for (i = 0; i < 256; ++i)
    encoding[i] = NULL;
  for (i = 0; i < gfxFontEncHashSize; ++i)
    hashTab[i] = -1;
}

GfxFontEncoding::GfxFontEncoding(char **encoding1, int encSize) {
  int i;

  encoding = encoding1;
  freeEnc = gFalse;
  for (i = 0; i < gfxFontEncHashSize; ++i)
    hashTab[i] = -1;
  for (i = 0; i < encSize; ++i) {
    if (encoding[i])
      addChar1(i, encoding[i]);
  }
}

void GfxFontEncoding::addChar(int code, char *name) {
  int h, i;

  // replace character associated with code
  if (encoding[code]) {
    h = hash(encoding[code]);
    for (i = 0; i < gfxFontEncHashSize; ++i) {
      if (hashTab[h] == code) {
	hashTab[h] = -2;
	break;
      }
      if (++h == gfxFontEncHashSize)
	h = 0;
    }
    gfree(encoding[code]);
  }

  // associate name with code
  encoding[code] = name;

  // insert name in hash table
  addChar1(code, name);
}

void GfxFontEncoding::addChar1(int code, char *name) {
  int h, i, code2;

  // insert name in hash table
  h = hash(name); 
  for (i = 0; i < gfxFontEncHashSize; ++i) {
    code2 = hashTab[h];
    if (code2 < 0) {
      hashTab[h] = code;
      break;
    } else if (!strcmp(encoding[code2], name)) {
      // keep the highest code for each char -- this is needed because
      // X won't display chars with codes < 32
      if (code > code2)
	hashTab[h] = code;
      break;
    }
    if (++h == gfxFontEncHashSize)
      h = 0;
  }
}

GfxFontEncoding::~GfxFontEncoding() {
  int i;

  if (freeEnc) {
    for (i = 0; i < 256; ++i) {
      if (encoding[i])
	gfree(encoding[i]);
    }
    gfree(encoding);
  }
}

int GfxFontEncoding::getCharCode(char *name) {
  int h, i, code;

  h = hash(name);
  for (i = 0; i < gfxFontEncHashSize; ++i) {
    code = hashTab[h];
    if (code == -1 || (code > 0 && !strcmp(encoding[code], name)))
      return code;
    if (++h > gfxFontEncHashSize)
      h = 0;
  }
  return -1;
}

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

GfxFont::GfxFont(char *tag1, Ref id1, Dict *fontDict) {
  BuiltinFont *builtinFont;
  char buf[256];
  Object obj1, obj2, obj3;
  char *p1, *p2;
  int i;

  // get font tag and ID
  tag = new GString(tag1);
  id = id1;

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

  // get info from font descriptor
  flags = fontSerif;   // assume Times-Roman by default
  embFontID.num = -1;
  embFontID.gen = -1;
  embFontName = NULL;
  fontDict->lookup("FontDescriptor", &obj1);
  if (obj1.isDict()) {

    // flags
    obj1.dictLookup("Flags", &obj2);
    if (obj2.isInt())
      flags = obj2.getInt();
    obj2.free();

    // embedded font file and font name
    obj1.dictLookupNF("FontFile", &obj2);
    if (obj2.isRef()) {
      embFontID = obj2.getRef();

      // get font name from the font file itself since font subsets
      // sometimes use the 'AAAAAA+foo' name and sometimes use just 'foo'
      obj2.fetch(&obj3);
      if (obj3.isStream()) {
	obj3.streamReset();
	for (i = 0; i < 64; ++i) {
	  obj3.streamGetLine(buf, sizeof(buf));
	  if (!strncmp(buf, "/FontName", 9)) {
	    if ((p1 = strchr(buf+9, '/'))) {
	      ++p1;
	      for (p2 = p1; *p2 && !isspace(*p2); ++p2) ;
	      embFontName = new GString(p1, p2 - p1);
	    }
	    break;
	  }
	}
      }
      obj3.free();
      obj2.free();

      // couldn't find font name so just use the one in the PDF font
      // descriptor
      if (!embFontName) {
	obj1.dictLookup("FontName", &obj2);
	if (obj2.isName())
	  embFontName = new GString(obj2.getName());
      }
    }
    obj2.free();
  }
  obj1.free();

  // get encoding and character widths
  if (builtinFont) {
    makeEncoding(fontDict, builtinFont->encoding);
    makeWidths(fontDict, builtinFont->encoding, builtinFont->widths);
  } else {
    makeEncoding(fontDict, NULL);
    makeWidths(fontDict, NULL, NULL);
  }
}

double GfxFont::getWidth(GString *s) {
  double w;
  int i;

  w = 0;
  for (i = 0; i < s->getLength(); ++i)
    w += widths[s->getChar(i) & 0xff];
  return w;
}

void GfxFont::makeEncoding(Dict *fontDict, GfxFontEncoding *builtinEncoding) {
  GfxFontEncoding *baseEnc;
  Object obj1, obj2, obj3;
  char *name;
  GBool hex;
  int code, n, i;

  // start with empty encoding
  encoding = new GfxFontEncoding();

  // get encoding from font dict
  fontDict->lookup("Encoding", &obj1);

  // encoding specified by dictionary
  if (obj1.isDict()) {
    obj1.dictLookup("BaseEncoding", &obj2);
    baseEnc = makeEncoding1(obj2, fontDict, builtinEncoding);
    obj2.free();
    obj1.dictLookup("Differences", &obj2);
    if (obj2.isArray()) {
      code = 0;
      for (i = 0; i < obj2.arrayGetLength(); ++i) {
	obj2.arrayGet(i, &obj3);
	if (obj3.isInt()) {
	  code = obj3.getInt();
	} else if (obj3.isName()) {
	  if (code < 256)
	    encoding->addChar(code, copyString(obj3.getName()));
	  ++code;
	} else {
	  error(-1, "Wrong type in font encoding resource differences (%s)",
		obj3.getTypeName());
	}
	obj3.free();
      }
    }
    obj2.free();

  // encoding specified by name or null
  } else {
    baseEnc = makeEncoding1(obj1, fontDict, builtinEncoding);
  }

  // free the font dict encoding
  obj1.free();

  // merge base encoding and differences;
  // try to fix decimal or hex character names for font subsets
  hex = gFalse;
  for (code = 0; code < 256; ++code) {
    if ((name = encoding->getCharName(code))) {
      if ((name[0] == 'C' || name[0] == 'G') && strlen(name) == 3 &&
	  ((name[1] >= 'a' && name[1] <= 'f') ||
	   (name[1] >= 'A' && name[1] <= 'F') ||
	   (name[2] >= 'a' && name[2] <= 'f') ||
	   (name[2] >= 'A' && name[2] <= 'F'))) {
	hex = gTrue;
	break;
      }
    }
  }
  for (code = 0; code < 256; ++code) {
    if ((name = encoding->getCharName(code))) {
      n = strlen(name);
      i = -1;
      if (hex && n == 3 && (name[0] == 'C' || name[0] == 'G') &&
	  isxdigit(name[1]) && isxdigit(name[2]))
	sscanf(name+1, "%x", &i);
      else if (!hex && n >= 3 && n <= 5 && isdigit(name[1]))
	i = atoi(name+1);
      if (i >= 0 && i < winAnsiEncodingSize) {
	//~ this is a kludge -- is there a standard internal encoding
	//~ used by all/most Type 1 fonts?
	if (i == 262)		// hyphen
	  i = 45;
	else if (i == 266)	// emdash
	  i = 208;
	if ((name = winAnsiEncoding.getCharName(i)))
	  encoding->addChar(code, copyString(name));
      }
    } else if ((name = baseEnc->getCharName(code))) {
      encoding->addChar(code, copyString(name));
    }
  }
}

GfxFontEncoding *GfxFont::makeEncoding1(Object obj, Dict *fontDict,
					GfxFontEncoding *builtinEncoding) {
  GfxFontEncoding *enc;
  GBool isType1, haveEncoding;
  Object obj1, obj2;
  char **path;
  GString *fileName;
  FILE *f;
  FileStream *str;

  // MacRoman, WinAnsi, or Standard encoding
  if (obj.isName("MacRomanEncoding")) {
    enc = &macRomanEncoding;
  } else if (obj.isName("WinAnsiEncoding")) {
    enc = &winAnsiEncoding;
  } else if (obj.isName("StandardEncoding")) {
    enc = &standardEncoding;

  // use the built-in font encoding if possible
  } else if (builtinEncoding) {
    enc = builtinEncoding;

  // try to get encoding from Type 1 font file
  } else {
    // default to using standard encoding
    enc = &standardEncoding;

    // is it a Type 1 font?
    fontDict->lookup("Subtype", &obj1);
    isType1 = obj1.isName("Type1");
    obj1.free();
    if (isType1) {

      // is there an external font file?
      haveEncoding = gFalse;
      if (name) {
	for (path = fontPath; *path; ++path) {
	  fileName = appendToPath(new GString(*path), name->getCString());
	  f = fopen(fileName->getCString(), "r");
	  if (!f) {
	    fileName->append(".pfb");
	    f = fopen(fileName->getCString(), "r");
	  }
	  if (!f) {
	    fileName->del(fileName->getLength() - 4, 4);
	    fileName->append(".pfa");
	    f = fopen(fileName->getCString(), "r");
	  }
	  delete fileName;
	  if (f) {
	    obj1.initNull();
	    str = new FileStream(f, 0, -1, &obj1);
	    getType1Encoding(str);
	    delete str;
	    fclose(f);
	    haveEncoding = gTrue;
	    break;
	  }
	}
      }

      // is there an embedded font file?
      //~ this should be checked before the external font, but
      //~ XOutputDev needs the encoding from the external font
      if (!haveEncoding) {
	fontDict->lookup("FontDescriptor", &obj1);
	if (obj1.isDict()) {
	  if (obj1.dictLookup("FontFile", &obj2)->isStream())
	    getType1Encoding(obj2.getStream());
	  obj2.free();
	}
	obj1.free();
      }
    }
  }

  return enc;
}

void GfxFont::getType1Encoding(Stream *str) {
  char buf[256];
  char *p;
  GBool found;
  int code, i;

  // look for encoding in font file
  str->reset();
  found = gFalse;
  for (i = 0; i < 100; ++i) {
    if (!str->getLine(buf, sizeof(buf)))
      break;
    if (!strncmp(buf, "/Encoding StandardEncoding def", 30))
      break;
    if (!strncmp(buf, "/Encoding 256 array", 19)) {
      found = gTrue;
      break;
    }
  }

  // found the encoding, grab it
  if (found) {
    for (i = 0; i < 300; ++i) {
      if (!str->getLine(buf, sizeof(buf)))
	break;
      p = strtok(buf, " \t");
      if (p && !strcmp(p, "dup")) {
	if ((p = strtok(NULL, " \t"))) {
	  code = atoi(p);
	  if ((p = strtok(NULL, " \t"))) {
	    if (p[0] == '/')
	      encoding->addChar(code, copyString(p+1));
	  }
	}
      }
    }
    //~ look for getinterval/putinterval junk
  }
}

void GfxFont::makeWidths(Dict *fontDict, GfxFontEncoding *builtinEncoding,
			 Gushort *builtinWidths) {
  Object obj1, obj2;
  int firstChar, lastChar;
  int code, code2;
  char *name;
  Gushort *defWidths;
  int index;

  // initialize all widths to zero
  for (code = 0; code < 256; ++code)
    widths[code] = 0;

  // use widths from built-in font
  if (builtinEncoding) {
    code2 = 0; // to make gcc happy
    for (code = 0; code < 256; ++code) {
      if ((name = encoding->getCharName(code)) &&
	  (code2 = builtinEncoding->getCharCode(name)) >= 0)
	widths[code] = builtinWidths[code2] / 1000.0;
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
      for (code = firstChar; code <= lastChar; ++code) {
	obj1.arrayGet(code - firstChar, &obj2);
	if (obj2.isNum())
	  widths[code] = obj2.getNum() / 1000;
	obj2.free();
      }
    } else {
      error(-1, "No character widths resource for non-builtin font");
      if (isFixedWidth())
	index = 8;
      else if (isSerif())
	index = 4;
      else
	index = 0;
      if (isBold())
	index += 2;
      if (isItalic())
	index += 1;
      defWidths = defCharWidths[index];
      code2 = 0; // to make gcc happy
      for (code = 0; code < 256; ++code) {
	if ((name = encoding->getCharName(code)) &&
	    (code2 = standardEncoding.getCharCode(name)) >= 0)
	  widths[code] = defWidths[code2] / 1000.0;
      }
    }
    obj1.free();
  }
}

GfxFont::~GfxFont() {
  delete tag;
  if (name)
    delete name;
  if (encoding)
    delete encoding;
  if (embFontName)
    delete embFontName;
}

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

GfxFontDict::GfxFontDict(Dict *fontDict) {
  int i;
  Object obj1, obj2;

  numFonts = fontDict->getLength();
  fonts = (GfxFont **)gmalloc(numFonts * sizeof(GfxFont *));
  for (i = 0; i < numFonts; ++i) {
    fontDict->getValNF(i, &obj1);
    obj1.fetch(&obj2);
    if (obj1.isRef() && obj2.isDict("Font")) {
      fonts[i] = new GfxFont(fontDict->getKey(i), obj1.getRef(),
			     obj2.getDict());
    } else {
      error(-1, "font resource is not a dictionary");
      fonts[i] = NULL;
    }
    obj1.free();
    obj2.free();
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
