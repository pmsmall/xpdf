//========================================================================
//
// PSOutput.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef PSOUTPUT_H
#define PSOUTPUT_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <gtypes.h>

class Catalog;
class GfxFont;

//------------------------------------------------------------------------
// PSOutput
//------------------------------------------------------------------------

class PSOutput {
public:

  // Open a PostScript output file, and write the prolog.
  PSOutput(char *fileName, Catalog *catalog,
	   int firstPage, int lastPage);

  // Write the trailer and close the file.
  ~PSOutput();

  // Check if file was successfully created.
  GBool isOk() { return ok; }

  // Start a page.
  void startPage(int pageNum, int x1, int y1, int x2, int y2);

  // End a page.
  void endPage();

  // Write trailer at end of file.
  void trailer();

  // Write PostScript code to file.  Uses printf format.
  void writePS(char *fmt, ...);

  // Write a string to the file, using escape chars as necessary.
  void writePSString(GString *s);

  // Write an image to file, given its defining dictionary and
  // data stream.
  void writeImage(Dict *dict, Stream *str, GBool inlineImg);

private:

  void writeStream(Stream *str, GBool inlineImg, GBool needA85);

  FILE *f;			// PostScript file
  int seqPage;			// current sequential page number
  GBool ok;			// set up ok?

  void setupFont(GfxFont *font);
};

#endif
