//========================================================================
//
// pdftotext.cc
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "parseargs.h"
#include "GString.h"
#include "gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "TextOutputDev.h"
#include "Params.h"
#include "Error.h"
#include "config.h"

static void printInfoString(FILE *f, Dict *infoDict, char *key, char *fmt);
static void printInfoDate(FILE *f, Dict *infoDict, char *key, char *fmt);

static int firstPage = 1;
static int lastPage = 0;
static GBool useASCII7 = gFalse;
static GBool useLatin2 = gFalse;
static GBool useLatin5 = gFalse;
#if JAPANESE_SUPPORT
static GBool useEUCJP = gFalse;
#endif
static GBool rawOrder = gFalse;
static GBool htmlMeta = gFalse;
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
  {"-ascii7", argFlag,     &useASCII7,     0,
   "convert to 7-bit ASCII (default is 8-bit ISO Latin-1)"},
  {"-latin2", argFlag,     &useLatin2,     0,
   "convert to ISO Latin-2 character set"},
  {"-latin5", argFlag,     &useLatin5,     0,
   "convert to ISO Latin-5 character set"},
#if JAPANESE_SUPPORT
  {"-eucjp",  argFlag,     &useEUCJP,      0,
   "convert Japanese text to EUC-JP"},
#endif
  {"-raw",    argFlag,     &rawOrder,      0,
   "keep strings in content stream order"},
  {"-htmlmeta", argFlag,   &htmlMeta,      0,
   "generate a simple HTML file, including the meta information"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-q",      argFlag,     &errQuiet,      0,
   "don't print any messages or errors"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

int main(int argc, char *argv[]) {
  PDFDoc *doc;
  GString *fileName;
  GString *textFileName;
  GString *ownerPW, *userPW;
  TextOutputDev *textOut;
  TextOutputCharSet charSet;
  FILE *f;
  Object info;
  GBool ok;
  char *p;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftotext version %s\n", xpdfVersion);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftotext", "<PDF-file> [<text-file>]", argDesc);
    }
    exit(1);
  }
  fileName = new GString(argv[1]);

  // init error file
  errorInit();

  // read config file
  initParams(xpdfUserConfigFile, xpdfSysConfigFile);

  // open PDF file
  xref = NULL;
  if (ownerPassword[0]) {
    ownerPW = new GString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
    userPW = new GString(userPassword);
  } else {
    userPW = NULL;
  }
  doc = new PDFDoc(fileName, ownerPW, userPW);
  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    goto err1;
  }

  // check for copy permission
  if (!doc->okToCopy()) {
    error(-1, "Copying of text from this document is not allowed.");
    goto err1;
  }

  // construct text file name
  if (argc == 3) {
    textFileName = new GString(argv[2]);
  } else {
    p = fileName->getCString() + fileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
      textFileName = new GString(fileName->getCString(),
				 fileName->getLength() - 4);
    } else {
      textFileName = fileName->copy();
    }
    textFileName->append(htmlMeta ? ".html" : ".txt");
  }

  // get page range
  if (firstPage < 1) {
    firstPage = 1;
  }
  if (lastPage < 1 || lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }

  // write HTML header
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "w"))) {
	error(-1, "Couldn't open text file '%s'", textFileName->getCString());
	goto err2;
      }
    }
    fputs("<html>\n", f);
    fputs("<head>\n", f);
    doc->getDocInfo(&info);
    if (info.isDict()) {
      printInfoString(f, info.getDict(), "Title", "<title>%s</title>\n");
      printInfoString(f, info.getDict(), "Subject",
		      "<meta name=\"Subject\" content=\"%s\">\n");
      printInfoString(f, info.getDict(), "Keywords",
		      "<meta name=\"Keywords\" content=\"%s\">\n");
      printInfoString(f, info.getDict(), "Author",
		      "<meta name=\"Author\" content=\"%s\">\n");
      printInfoString(f, info.getDict(), "Creator",
		      "<meta name=\"Creator\" content=\"%s\">\n");
      printInfoString(f, info.getDict(), "Producer",
		      "<meta name=\"Producer\" content=\"%s\">\n");
      printInfoDate(f, info.getDict(), "CreationDate",
		    "<meta name=\"CreationDate\" content=\"%s\">\n");
      printInfoDate(f, info.getDict(), "LastModifiedDate",
		    "<meta name=\"ModDate\" content=\"%s\">\n");
    }
    info.free();
    fputs("</head>\n", f);
    fputs("<body>\n", f);
    fputs("<pre>\n", f);
    if (f != stdout) {
      fclose(f);
    }
  }

  // write text file
#if JAPANESE_SUPPORT
  useASCII7 |= useEUCJP;
#endif
  charSet = textOutLatin1;
  if (useASCII7) {
    charSet = textOutASCII7;
  } else if (useLatin2) {
    charSet = textOutLatin2;
  } else if (useLatin5) {
    charSet = textOutLatin5;
  }
  textOut = new TextOutputDev(textFileName->getCString(), charSet, rawOrder,
			      htmlMeta);
  if (textOut->isOk()) {
    doc->displayPages(textOut, firstPage, lastPage, 72, 0, gFalse);
  }
  delete textOut;

  // write end of HTML file
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "a"))) {
	error(-1, "Couldn't open text file '%s'", textFileName->getCString());
	goto err2;
      }
    }
    fputs("</pre>\n", f);
    fputs("</body>\n", f);
    fputs("</html>\n", f);
    if (f != stdout) {
      fclose(f);
    }
  }

  // clean up
 err2:
  delete textFileName;
 err1:
  delete doc;
  freeParams();

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}

static void printInfoString(FILE *f, Dict *infoDict, char *key, char *fmt) {
  Object obj;
  GString *s1, *s2;
  int i;

  if (infoDict->lookup(key, &obj)->isString()) {
    s1 = obj.getString();
    if ((s1->getChar(0) & 0xff) == 0xfe &&
	(s1->getChar(1) & 0xff) == 0xff) {
      s2 = new GString();
      for (i = 2; i < obj.getString()->getLength(); i += 2) {
	if (s1->getChar(i) == '\0') {
	  s2->append(s1->getChar(i+1));
	} else {
	  delete s2;
	  s2 = new GString("<unicode>");
	  break;
	}
      }
      fprintf(f, fmt, s2->getCString());
      delete s2;
    } else {
      fprintf(f, fmt, s1->getCString());
    }
  }
  obj.free();
}

static void printInfoDate(FILE *f, Dict *infoDict, char *key, char *fmt) {
  Object obj;
  char *s;

  if (infoDict->lookup(key, &obj)->isString()) {
    s = obj.getString()->getCString();
    if (s[0] == 'D' && s[1] == ':') {
      s += 2;
    }
    fprintf(f, fmt, s);
  }
  obj.free();
}
