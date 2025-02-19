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
#include <parseargs.h>
#include <GString.h>
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "Link.h"
#include "PSOutput.h"
#include "Flags.h"
#include "Error.h"
#include "config.h"

static void writePostScript(GString *psFileName);
static GBool loadFile(GString *fileName);

static int firstPage = 1;
static int lastPage = 0;
GBool printCommands = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to print"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to print"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

static FILE *file;
static Catalog *catalog;

int main(int argc, char *argv[]) {
  GBool ok;
  GString *fileName, *psFileName;
  char *p;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printHelp) {
    printUsage("pdftops", "<PDF-file> [<PS-file>]", argDesc);
    exit(1);
  }

  // init error file
  errorInit();

  // load PDF file
  fileName = new GString(argv[1]);
  if (!loadFile(fileName)) {
    delete fileName;
    exit(1);
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
    psFileName->append(".ps");
  }

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (lastPage < 1 || lastPage > catalog->getNumPages())
    lastPage = catalog->getNumPages();

  // write PostScript file
  writePostScript(psFileName);

  // clean up
  delete catalog;
  delete xref;
  fclose(file);
  delete fileName;
  delete psFileName;

  // check for memory leaks
  Object::memCheck(errFile);
}

static GBool loadFile(GString *fileName) {
  FileStream *str;
  Object catObj;
  Object obj;

  // no xref yet
  xref = NULL;

  // open PDF file and create stream
  if (!(file = fopen(fileName->getCString(), "r"))) {
    error(0, "Couldn't open file '%s'", fileName->getCString());
    goto err1;
  }
  obj.initNull();
  str = new FileStream(file, 0, -1, &obj);

  // check header
  str->checkHeader();

  // read xref table
  xref = new XRef(str);
  delete str;
  if (!xref->isOk()) {
    error(0, "Couldn't read xref table");
    goto err2;
  }
  if (xref->checkEncrypted())
    goto err2;

  // read catalog
  catalog = new Catalog(xref->getCatalog(&catObj));
  catObj.free();
  if (!catalog->isOk()) {
    error(0, "Couldn't read page catalog");
    goto err3;
  }

  // done
  return gTrue;

 err3:
  delete catalog;
 err2:
  delete xref;
  fclose(file);
 err1:
  return gFalse;
}

static void writePostScript(GString *psFileName) {
  PSOutput *psOut;
  int pg;

  psOut = new PSOutput(psFileName->getCString(), catalog,
		       firstPage, lastPage);
  if (psOut->isOk()) {
    for (pg = firstPage; pg <= lastPage; ++pg)
      catalog->getPage(pg)->genPostScript(psOut, 72, 0);
  }
  delete psOut;
}
