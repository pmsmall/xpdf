//========================================================================
//
// pdftops.cc
//
// Copyright 1996 Derek B. Noonburg
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
#include "PSOutputDev.h"
#include "Params.h"
#include "Error.h"
#include "config.h"

static int firstPage = 1;
static int lastPage = 0;
static GBool noEmbedFonts = gFalse;
static GBool noEmbedTTFonts = gFalse;
static GBool doForm = gFalse;
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,      0,
   "first page to print"},
  {"-l",      argInt,      &lastPage,       0,
   "last page to print"},
  {"-paperw", argInt,      &paperWidth,     0,
   "paper width, in points"},
  {"-paperh", argInt,      &paperHeight,    0,
   "paper height, in points"},
  {"-level1", argFlag,     &psOutLevel1,    0,
   "generate Level 1 PostScript"},
  {"-level1sep", argFlag,  &psOutLevel1Sep, 0,
   "generate Level 1 separable PostScript"},
  {"-level2sep", argFlag,  &psOutLevel2Sep, 0,
   "generate Level 2 separable PostScript"},
  {"-eps",    argFlag,     &psOutEPS,       0,
   "generate Encapsulated PostScript (EPS)"},
#if OPI_SUPPORT
  {"-opi",    argFlag,     &psOutOPI,       0,
   "generate OPI comments"},
#endif
  {"-noemb",  argFlag,     &noEmbedFonts,   0,
   "don't embed Type 1 fonts"},
  {"-noembtt", argFlag,    &noEmbedTTFonts, 0,
   "don't embed TrueType fonts"},
  {"-form",   argFlag,     &doForm,         0,
   "generate a PostScript form"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,    sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-q",      argFlag,     &errQuiet,       0,
   "don't print any messages or errors"},
  {"-v",      argFlag,     &printVersion,   0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,      0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,      0,
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
  GString *psFileName;
  GString *ownerPW, *userPW;
  PSOutputDev *psOut;
  GBool ok;
  char *p;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftops version %s\n", xpdfVersion);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftops", "<PDF-file> [<PS-file>]", argDesc);
    }
    exit(1);
  }
  if ((psOutLevel1 && psOutLevel1Sep) ||
      (psOutLevel1Sep && psOutLevel2Sep) ||
      (psOutLevel1 && psOutLevel2Sep)) {
    fprintf(stderr, "Error: use only one of -level1, -level1sep, and -level2sep.\n");
    exit(1);
  }
  if (doForm && (psOutLevel1 || psOutLevel1Sep)) {
    fprintf(stderr, "Error: forms are only available with Level 2 output.\n");
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

  // check for print permission
  if (!doc->okToPrint()) {
    error(-1, "Printing this document is not allowed.");
    goto err1;
  }

  // construct PostScript file name
  if (argc == 3) {
    psFileName = new GString(argv[2]);
  } else {
    p = fileName->getCString() + fileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF"))
      psFileName = new GString(fileName->getCString(),
			       fileName->getLength() - 4);
    else
      psFileName = fileName->copy();
    psFileName->append(psOutEPS ? ".eps" : ".ps");
  }

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();
  if (doForm)
    lastPage = firstPage;

  // check for multi-page EPS
  if (psOutEPS && firstPage != lastPage) {
    error(-1, "EPS files can only contain one page.");
    goto err2;
  }

  // write PostScript file
  psOut = new PSOutputDev(psFileName->getCString(), doc->getCatalog(),
			  firstPage, lastPage,
			  !noEmbedFonts, !noEmbedTTFonts, doForm);
  if (psOut->isOk())
    doc->displayPages(psOut, firstPage, lastPage, 72, 0, gFalse);
  delete psOut;

  // clean up
 err2:
  delete psFileName;
 err1:
  delete doc;
  freeParams();

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}
