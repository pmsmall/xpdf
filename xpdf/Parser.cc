//========================================================================
//
// Parser.cc
//
//========================================================================

#pragma implementation

#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Parser.h"
#include "Error.h"

Parser::Parser(Lexer *lexer1) {
  lexer = lexer1;
  lexer->getObj(&buf1);
  lexer->getObj(&buf2);
}

Parser::~Parser() {
  buf1.free();
  buf2.free();
  delete lexer;
}

Object *Parser::getObj(Object *obj) {
  char *key;
  Stream *str;
  Object obj2;
  int num;

  // array
  if (buf1.isCmd("[")) {
    shift();
    obj->initArray();
    while (!buf1.isCmd("]") && !buf1.isEOF())
      obj->arrayAdd(getObj(&obj2));
    if (buf1.isEOF())
      error(getPos(), "End of file inside array");
    shift();

  // dictionary or stream
  } else if (buf1.isCmd("<<")) {
    shift();
    obj->initDict();
    while (!buf1.isCmd(">>") && !buf1.isEOF()) {
      if (!buf1.isName()) {
	error(getPos(), "Dictionary key must be a name object");
	shift();
      } else {
	key = copyString(buf1.getName());
	shift();
	if (buf1.isEOF() || buf1.isError())
	  break;
	obj->dictAdd(key, getObj(&obj2));
      }
    }
    if (buf1.isEOF())
      error(getPos(), "End of file inside dictionary");
    if (buf2.isCmd("stream")) {
      if (str = makeStream(obj)) {
	obj->initStream(str);
      } else {
	obj->free();
	obj->initError();
      }
    } else {
      shift();
    }

  // indirect reference or integer
  } else if (buf1.isInt()) {
    num = buf1.getInt();
    shift();
    if (buf1.isInt() && buf2.isCmd("R")) {
      obj->initRef(num, buf1.getInt());
      shift();
      shift();
    } else {
      obj->initInt(num);
    }

  // simple object
  } else {
    buf1.copy(obj);
    shift();
  }

  return obj;
}

Stream *Parser::makeStream(Object *dict) {
  Object obj, obj2;
  Object params, params2;
  Stream *str;
  int pos, length;
  int i;

  // get stream start position
  lexer->skipToNextLine();
  pos = lexer->getPos() - 1;

  // get length
  dict->dictLookup("Length", &obj);
  if (obj.isInt()) {
    length = obj.getInt();
    obj.free();
  } else {
    error(getPos(), "Bad 'Length' attribute in stream");
    obj.free();
    return NULL;
  }

  // make base stream
  str = new FileStream(lexer->getStream()->getFile(), pos, length, dict);

  // get filters
  dict->dictLookup("Filter", &obj);
  if (obj.isName()) {
    dict->dictLookup("DecodeParms", &params);
    str = makeFilter(obj.getName(), str, &params);
    params.free();
  } else if (obj.isArray()) {
    dict->dictLookup("DecodeParms", &params);
    for (i = 0; i < obj.arrayGetLength(); ++i) {
      obj.arrayGet(i, &obj2);
      if (params.isArray())
	params.arrayGet(i, &params2);
      else
	params2.initNull();
      if (obj2.isName())
	str = makeFilter(obj2.getName(), str, &params2);
      else
	error(getPos(), "Bad filter name");
      obj2.free();
      params2.free();
    }
    params.free();
  } else if (!obj.isNull()) {
    error(getPos(), "Bad 'Filter' attribute in stream");
  }
  obj.free();

  // skip over stream data
  lexer->getStream()->setPos(pos + length);

  // refill token buffers and check for 'endstream'
  shift();  // kill '>>'
  shift();  // kill 'stream'
  if (buf1.isCmd("endstream"))
    shift();
  else
    error(getPos(), "Missing 'endstream'");

  return str;
}

Stream *Parser::makeFilter(char *name, Stream *str, Object *params) {
  int predictor;		// parameters
  int colors;
  int bits;
  int early;
  int encoding;
  Boolean byteAlign;
  Boolean black;
  int columns, rows;
  Object obj;

  if (!strcmp(name, "ASCIIHexDecode")) {
    str = new ASCIIHexStream(str);
  } else if (!strcmp(name, "ASCII85Decode")) {
    str = new ASCII85Stream(str);
  } else if (!strcmp(name, "LZWDecode")) {
    predictor = 1;
    columns = 1;
    colors = 1;
    bits = 8;
    early = 1;
    if (params->isDict()) {
      params->dictLookup("Predictor", &obj);
      if (obj.isInt())
	predictor = obj.getInt();
      obj.free();
      params->dictLookup("Columns", &obj);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Colors", &obj);
      if (obj.isInt())
	colors = obj.getInt();
      obj.free();
      params->dictLookup("BitsPerComponent", &obj);
      if (obj.isInt())
	bits = obj.getInt();
      obj.free();
      params->dictLookup("EarlyChange", &obj);
      if (obj.isInt())
	early = obj.getInt();
      obj.free();
    }
    str = new LZWStream(str, predictor, columns, colors, bits, early);
//~  } else if (!strcmp(name, "RunLengthDecode")) {
  } else if (!strcmp(name, "CCITTFaxDecode")) {
    encoding = 0;
    byteAlign = false;
    columns = 1728;
    rows = 0;
    black = false;
    if (params->isDict()) {
      params->dictLookup("K", &obj);
      if (obj.isInt())
	encoding = obj.getInt();
      obj.free();
      params->dictLookup("EncodedByteAlign", &obj);
      if (obj.isBool())
	byteAlign = obj.getBool();
      obj.free();
      params->dictLookup("Columns", &obj);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Rows", &obj);
      if (obj.isInt())
	rows = obj.getInt();
      obj.free();
      params->dictLookup("BlackIs1", &obj);
      if (obj.isBool())
	black = obj.getBool();
      obj.free();
    }
    str = new CCITTFaxStream(str, encoding, byteAlign, columns, rows, black);
//~  } else if (!strcmp(name, "DCTDecode")) {
  } else {
    error(getPos(), "Unknown filter '%s'", name);
  }
  return str;
}

void Parser::shift() {
  buf1.free();
  buf1 = buf2;
  lexer->getObj(&buf2);
}
