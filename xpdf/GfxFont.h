//========================================================================
//
// GfxFont.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef GFXFONT_H
#define GFXFONT_H

#ifdef __GNUC__
#pragma interface
#endif

#include <gtypes.h>
#include <GString.h>

class Dict;

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

#define fontFixedWidth (1 << 0)
#define fontSerif      (1 << 1)
#define fontSymbolic   (1 << 2)
#define fontItalic     (1 << 6)
#define fontBold       (1 << 18)

class GfxFont {
public:

  // Constructor.
  GfxFont(char *tag1, Dict *fontDict);

  // Destructor.
  ~GfxFont();

  // Get font tag.
  GString *getTag() { return tag; }

  // Does this font match the tag?
  GBool matches(char *tag1) { return !tag->cmp(tag1); }

  // Get base font name.
  GString *getName() { return name; }

  // Get font descriptor flags.
  GBool isFixedWidth() { return flags & fontFixedWidth; }
  GBool isSerif() { return flags & fontSerif; }
  GBool isSymbolic() { return flags & fontSymbolic; }
  GBool isItalic() { return flags & fontItalic; }
  GBool isBold() { return flags & fontBold; }

  // Get width of a character.
  double getWidth(Guchar c) { return widths[c]; }
  double getWidth(GString *s);

  // Get encoded character name.
  char *getCharName(int code) { return encoding[code]; }

  // Look up a character name in an encoding.
  int lookupCharName(char *name, char **enc, int encSize, int hint);

private:

  void makeEncoding(Dict *fontDict, char **builtinEncoding);
  void makeWidths(Dict *fontDict, char **builtinEncoding,
		  int builtinEncodingSize, Gushort *builtinWidths);

  GString *tag;
  GString *name;
  int flags;
  double widths[256];
  char *encoding[256];
};

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

class GfxFontDict {
public:

  // Build the font dictionary, given the PDF font dictionary.
  GfxFontDict(Dict *fontDict);

  // Destructor.
  ~GfxFontDict();

  // Get the specified font.
  GfxFont *lookup(char *tag);

  // Iterative access.
  int getNumFonts() { return numFonts; }
  GfxFont *getFont(int i) { return fonts[i]; }

private:

  GfxFont **fonts;		// list of fonts
  int numFonts;			// number of fonts
};

#endif
