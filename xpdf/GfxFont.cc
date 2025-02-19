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
#include "config.h"
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
    } else if (encoding[code2] && !strcmp(encoding[code2], name)) {
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
    if (code == -1 ||
	(code > 0 && encoding[code] && !strcmp(encoding[code], name)))
      return code;
    if (++h >= gfxFontEncHashSize)
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

  // get font type
  type = fontUnknownType;
  fontDict->lookup("Subtype", &obj1);
  if (obj1.isName("Type1"))
    type = fontType1;
  else if (obj1.isName("Type3"))
    type = fontType3;
  else if (obj1.isName("TrueType"))
    type = fontTrueType;
  obj1.free();

  // get info from font descriptor
  // for flags: assume Times-Roman (or TimesNewRoman), but
  // explicitly check for Arial and CourierNew -- certain PDF
  // generators apparently don't include FontDescriptors for Arial,
  // TimesNewRoman, and CourierNew
  flags = fontSerif;   // assume Times-Roman by default
  if (type == fontTrueType && !name->cmp("Arial"))
    flags = 0;
  else if (type == fontTrueType && !name->cmp("CourierNew"))
    flags = fontFixedWidth;
  embFontID.num = -1;
  embFontID.gen = -1;
  embFontName = NULL;
  extFontFile = NULL;
  fontDict->lookup("FontDescriptor", &obj1);
  if (obj1.isDict()) {

    // flags
    obj1.dictLookup("Flags", &obj2);
    if (obj2.isInt())
      flags = obj2.getInt();
    obj2.free();

    // embedded Type 1 font file and font name
    if (type == fontType1) {
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

    // embedded TrueType font file
    } else if (type == fontTrueType) {
      obj1.dictLookupNF("FontFile2", &obj2);
      if (obj2.isRef())
	embFontID = obj2.getRef();
      obj2.free();
    }
  }
  obj1.free();

  // get font matrix
  fontMat[0] = fontMat[3] = 1;
  fontMat[1] = fontMat[2] = fontMat[4] = fontMat[5] = 0;
  if (fontDict->lookup("FontMatrix", &obj1)->isArray()) {
    for (i = 0; i < 6 && i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum())
	fontMat[i] = obj2.getNum();
      obj2.free();
    }
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
  char *charName;
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
    if ((charName = encoding->getCharName(code))) {
      if ((charName[0] == 'C' || charName[0] == 'G') &&
	  strlen(charName) == 3 &&
	  ((charName[1] >= 'a' && charName[1] <= 'f') ||
	   (charName[1] >= 'A' && charName[1] <= 'F') ||
	   (charName[2] >= 'a' && charName[2] <= 'f') ||
	   (charName[2] >= 'A' && charName[2] <= 'F'))) {
	hex = gTrue;
	break;
      }
    }
  }
  for (code = 0; code < 256; ++code) {
    if ((charName = encoding->getCharName(code))) {
      n = strlen(charName);
      i = -1;
      if (hex && n == 3 && (charName[0] == 'C' || charName[0] == 'G') &&
	  isxdigit(charName[1]) && isxdigit(charName[2]))
	sscanf(charName+1, "%x", &i);
      else if (!hex && n >= 2 && n <= 3 &&
	       isdigit(charName[0]) && isdigit(charName[1]))
	i = atoi(charName);
      else if (!hex && n >= 3 && n <= 5 && isdigit(charName[1]))
	i = atoi(charName+1);
      if (i >= 0 && i < winAnsiEncodingSize) {
	//~ this is a kludge -- is there a standard internal encoding
	//~ used by all/most Type 1 fonts?
	if (i == 262)		// hyphen
	  i = 45;
	else if (i == 266)	// emdash
	  i = 208;
	if ((charName = winAnsiEncoding.getCharName(i)))
	  encoding->addChar(code, copyString(charName));
      }
    } else if ((charName = baseEnc->getCharName(code))) {
      encoding->addChar(code, copyString(charName));
    }
  }
}

GfxFontEncoding *GfxFont::makeEncoding1(Object obj, Dict *fontDict,
					GfxFontEncoding *builtinEncoding) {
  GfxFontEncoding *enc;
  GBool haveEncoding;
  Object obj1, obj2;
  char **path;
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

  // check font type
  } else {
    // Type 1 font: try to get encoding from font file
    if (type == fontType1) {

      // default to using standard encoding
      enc = &standardEncoding;

      // is there an external font file?
      haveEncoding = gFalse;
      if (name) {
	for (path = fontPath; *path; ++path) {
	  extFontFile = appendToPath(new GString(*path), name->getCString());
	  f = fopen(extFontFile->getCString(), FOPEN_READ_BIN);
	  if (!f) {
	    extFontFile->append(".pfb");
	    f = fopen(extFontFile->getCString(), FOPEN_READ_BIN);
	  }
	  if (!f) {
	    extFontFile->del(extFontFile->getLength() - 4, 4);
	    extFontFile->append(".pfa");
	    f = fopen(extFontFile->getCString(), FOPEN_READ_BIN);
	  }
	  if (f) {
	    obj1.initNull();
	    str = new FileStream(f, 0, -1, &obj1);
	    getType1Encoding(str);
	    delete str;
	    fclose(f);
	    haveEncoding = gTrue;
	    break;
	  }
	  delete extFontFile;
	  extFontFile = NULL;
	}
      }

      // is there an embedded font file?
      // (this has to be checked after the external font because
      // XOutputDev needs the encoding from the external font)
      if (!haveEncoding && embFontID.num >= 0) {
	obj1.initRef(embFontID.num, embFontID.gen);
	obj1.fetch(&obj2);
	if (obj2.isStream())
	  getType1Encoding(obj2.getStream());
	obj2.free();
	obj1.free();
      }

    // TrueType font: use Mac encoding
    } else if (type == fontTrueType) {
      enc = &macRomanEncoding;

    // not Type 1 or TrueType: just use the standard encoding
    } else {
      enc = &standardEncoding;
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
  char *charName;
  Gushort *defWidths;
  int index;
  double mult;

  // initialize all widths to zero
  for (code = 0; code < 256; ++code)
    widths[code] = 0;

  // use widths from built-in font
  if (builtinEncoding) {
    code2 = 0; // to make gcc happy
    for (code = 0; code < 256; ++code) {
      if ((charName = encoding->getCharName(code)) &&
	  (code2 = builtinEncoding->getCharCode(charName)) >= 0)
	widths[code] = builtinWidths[code2] * 0.001;
    }

  // get widths from font dict
  } else {
    fontDict->lookup("FirstChar", &obj1);
    firstChar = obj1.isInt() ? obj1.getInt() : 0;
    obj1.free();
    fontDict->lookup("LastChar", &obj1);
    lastChar = obj1.isInt() ? obj1.getInt() : 255;
    obj1.free();
    if (type == fontType3)
      mult = fontMat[0];
    else
      mult = 0.001;
    fontDict->lookup("Widths", &obj1);
    if (obj1.isArray()) {
      for (code = firstChar; code <= lastChar; ++code) {
	obj1.arrayGet(code - firstChar, &obj2);
	if (obj2.isNum())
	  widths[code] = obj2.getNum() * mult;
	obj2.free();
      }
    } else {

      // couldn't find widths -- use defaults 
#if 0 //~ certain PDF generators apparently don't include widths
      //~ for Arial and TimesNewRoman -- and this error message
      //~ is a nuisance
      error(-1, "No character widths resource for non-builtin font");
#endif
      if (isFixedWidth())
	index = 0;
      else if (isSerif())
	index = 8;
      else
	index = 4;
      if (isBold())
	index += 2;
      if (isItalic())
	index += 1;
      defWidths = defCharWidths[index];
      code2 = 0; // to make gcc happy
      for (code = 0; code < 256; ++code) {
	if ((charName = encoding->getCharName(code)) &&
	    (code2 = standardEncoding.getCharCode(charName)) >= 0)
	  widths[code] = defWidths[code2] * 0.001;
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
  if (extFontFile)
    delete extFontFile;
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
