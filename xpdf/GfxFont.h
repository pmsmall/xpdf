//========================================================================
//
// GfxFont.h
//
//========================================================================

#ifndef GFXFONT_H
#define GFXFONT_H

#pragma interface

#include <stypes.h>
#include <String.h>

class Dict;

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

#define fontFixedWidth (1 << 0)
#define fontSerif      (1 << 1)
#define fontItalic     (1 << 6)
#define fontBold       (1 << 18)

class GfxFont {
public:

  // Constructor.
  GfxFont(char *tag1, Dict *fontDict);

  // Destructor.
  ~GfxFont();

  // Does this font match the tag?
  Boolean matches(char *tag1) { return !tag->cmp(tag1); }

  // Get base font name.
  String *getName() { return name; }

  // Get font descriptor flags.
  Boolean isFixedWidth() { return flags & fontFixedWidth; }
  Boolean isSerif() { return flags & fontSerif; }
  Boolean isItalic() { return flags & fontItalic; }
  Boolean isBold() { return flags & fontBold; }

  // Get width of a character.
  double getWidth(uchar c) { return widths[c]; }

  // Get the mapping from font encoding to ISO encoding.
  ushort *getISOMap() { return isoMap; }

  // Get the mapping from ISO encoding to font encoding.
  ushort *getReverseISOMap() { return revISOMap; }

private:

  void getEncoding(Dict *dict);
  void findNamedChar(char *name, ushort *isoCode, ushort *pdfCode);

  String *tag;
  String *name;
  int flags;
  double widths[256];
  ushort isoMap[256];
  ushort revISOMap[256];
  ushort pdfMap[256];
};

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

class GfxFontDict {
public:

  GfxFontDict(Dict *fontDict);

  ~GfxFontDict();

  GfxFont *lookup(char *tag);

private:

  GfxFont **fonts;		// list of fonts
  int numFonts;			// number of fonts
};

#endif
