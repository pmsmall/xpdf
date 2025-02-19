//========================================================================
//
// FTFont.cc
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#if HAVE_FREETYPE_FREETYPE_H | HAVE_FREETYPE_H
#if FREETYPE2

#include <math.h>
#include <string.h>
#include "gmem.h"
#include "freetype/internal/ftobjs.h"
#include "freetype/internal/t1types.h"
#include "freetype/internal/cfftypes.h"
#include "freetype/internal/tttypes.h"
#include "FontEncoding.h"
#include "FTFont.h"

typedef TT_Face CFF_Face;
extern "C" int CFF_Find_Char(CFF_Face face, char *name);

//------------------------------------------------------------------------

FTFontEngine::FTFontEngine(Display *display, Visual *visual, int depth,
			   Colormap colormap, GBool aa):
  SFontEngine(display, visual, depth, colormap) {

  ok = gFalse;
  if (FT_Init_FreeType(&lib)) {
    return;
  }
  this->aa = aa;
  ok = gTrue;
}

FTFontEngine::~FTFontEngine() {
  FT_Done_FreeType(lib);
}

//------------------------------------------------------------------------

FTFontFile::FTFontFile(FTFontEngine *engine, char *fontFileName,
		       FontEncoding *fontEnc) {
  T1_Face t1Face;
  char *name1, *name2;
  int i, j;

  ok = gFalse;
  this->engine = engine;
#if 0 //~
  int err;
  if ((err = FT_New_Face(engine->lib, fontFileName, 0, &face))) {
    fprintf(stderr, "failed at FT_New_Face (%d %x)\n", err, err);
    return;
  }
#else
  if (FT_New_Face(engine->lib, fontFileName, 0, &face)) {
    return;
  }
#endif
#if 0 //~
  printf("FT module = %s\n", face->driver->root.clazz->module_name);
#endif

  if (!strcmp(face->driver->root.clazz->module_name, "type1")) {

#if 0 //~
    printf("FT2: type1\n");
#endif
    useGlyphMap = gTrue;
    t1Face = (T1_Face)face;
    for (i = 0; i < 256; ++i) {
      glyphMap[i] = 0;
      if ((name1 = fontEnc->getCharName(i))) {
	for (j = 0; j < t1Face->type1.num_glyphs; ++j) {
	  if ((name2 = t1Face->type1.glyph_names[j]) &&
	      !strcmp(name1, name2)) {
	    glyphMap[i] = j;
	    break;
	  }
	}
      }
    }

  } else if (!strcmp(face->driver->root.clazz->module_name, "cff")) {

#if 0 //~
    printf("FT2: type1c\n");
#endif
    useGlyphMap = gTrue;
    for (i = 0; i < 256; ++i) {
      glyphMap[i] = 0;
      if ((name1 = fontEnc->getCharName(i))) {
	glyphMap[i] = CFF_Find_Char((CFF_Face)face, name1);
      }
    }

  } else {

    useGlyphMap = gFalse;

    // Choose a cmap:
    // 1. If the font contains an Adobe cmap (which means it's a Type 1
    //    or Type 1C font), use it.
    // 2. If the font contains a Windows-symbol cmap, use it.
    // 3. Otherwise, use the first cmap in the TTF file.
    // 4. If the Windows-Symbol cmap is used (from either step 1 or step
    //    2), offset all character indexes by 0xf000.
    // This seems to match what acroread does, but may need further
    // tweaking.
#if 0 //~
    printf("available cmaps:\n");
    for (i = 0; i < face->num_charmaps; ++i) {
      printf("  %d: %d %d\n", i,
	     face->charmaps[i]->platform_id,
	     face->charmaps[i]->encoding_id);
    }
#endif
    for (i = 0; i < face->num_charmaps; ++i) {
      if (face->charmaps[i]->platform_id == 7) {
	break;
      }
    }
    if (i >= face->num_charmaps) {
      for (i = 0; i < face->num_charmaps; ++i) {
	if (face->charmaps[i]->platform_id == 3 &&
	    face->charmaps[i]->encoding_id == 0) {
	  break;
	}
      }
      if (i >= face->num_charmaps) {
	i = 0;
      }
    }
#if 0 //~
    printf("chose cmap %d\n", i);
#endif
    charMapOffset = 0;
    if (face->charmaps[i]->platform_id == 3 &&
	face->charmaps[i]->encoding_id == 0) {
      charMapOffset = 0xf000;
    }
    if (FT_Set_Charmap(face, face->charmaps[i])) {
#if 0 //~
      fprintf(stderr, "failed at FT_Set_Charmap\n");
#endif
      return;
    }
  }

  ok = gTrue;
}

FTFontFile::~FTFontFile() {
  if (face) {
    FT_Done_Face(face);
  }
}

//------------------------------------------------------------------------

FTFont::FTFont(FTFontFile *fontFile, double *m) {
  FTFontEngine *engine;
  FT_Face face;
  double size;
  int x, xMin, xMax;
  int y, yMin, yMax;
  int i;

  ok = gFalse;
  this->fontFile = fontFile;
  engine = fontFile->engine;
  face = fontFile->face;
  if (FT_New_Size(face, &sizeObj)) {
    return;
  }
  face->size = sizeObj;
  size = sqrt(m[2]*m[2] + m[3]*m[3]);
  if (FT_Set_Pixel_Sizes(face, 0, (int)size)) {
    return;
  }

  // transform the four corners of the font bounding box -- the min
  // and max values form the bounding box of the transformed font
  x = (int)((m[0] * face->bbox.xMin + m[2] * face->bbox.yMin) /
	    face->units_per_EM);
  xMin = xMax = x;
  y = (int)((m[1] * face->bbox.xMin + m[3] * face->bbox.yMin) /
	    face->units_per_EM);
  yMin = yMax = y;
  x = (int)((m[0] * face->bbox.xMin + m[2] * face->bbox.yMax) /
	    face->units_per_EM);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((m[1] * face->bbox.xMin + m[3] * face->bbox.yMax) /
	    face->units_per_EM);
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)((m[0] * face->bbox.xMax + m[2] * face->bbox.yMin) /
	    face->units_per_EM);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((m[1] * face->bbox.xMax + m[3] * face->bbox.yMin) /
	    face->units_per_EM);
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)((m[0] * face->bbox.xMax + m[2] * face->bbox.yMax) /
	    face->units_per_EM);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((m[1] * face->bbox.xMax + m[3] * face->bbox.yMax) /
	    face->units_per_EM);
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
#if 1 //~
  //~ This is a kludge: some buggy PDF generators embed fonts with
  //~ zero bounding boxes.
  if (xMax == xMin) {
    xMin = 0;
    xMax = (int)size;
  }
  if (yMax == yMin) {
    yMin = 0;
    yMax = (int)(1.2 * size);
  }
#endif
  glyphW = xMax - xMin + 1;
  glyphH = yMax - yMin + 1;
  if (engine->aa) {
    glyphSize = glyphW * glyphH;
  } else {
    glyphSize = ((glyphW + 7) >> 3) * glyphH;
  }

  // set up the glyph pixmap cache
  cacheAssoc = 8;
  if (glyphSize <= 256) {
    cacheSets = 8;
  } else if (glyphSize <= 512) {
    cacheSets = 4;
  } else if (glyphSize <= 1024) {
    cacheSets = 2;
  } else {
    cacheSets = 1;
  }
  cache = (Guchar *)gmalloc(cacheSets * cacheAssoc * glyphSize);
  cacheTags = (FTFontCacheTag *)gmalloc(cacheSets * cacheAssoc *
					sizeof(FTFontCacheTag));
  for (i = 0; i < cacheSets * cacheAssoc; ++i) {
    cacheTags[i].mru = i & (cacheAssoc - 1);
  }

  // create the XImage
  if (!(image = XCreateImage(engine->display, engine->visual, engine->depth,
			     ZPixmap, 0, NULL, glyphW, glyphH, 8, 0))) {
    return;
  }
  image->data = (char *)gmalloc(glyphH * image->bytes_per_line);

  // compute the transform matrix
  matrix.xx = (FT_Fixed)((m[0] / size) * 65536);
  matrix.yx = (FT_Fixed)((m[1] / size) * 65536);
  matrix.xy = (FT_Fixed)((m[2] / size) * 65536);
  matrix.yy = (FT_Fixed)((m[3] / size) * 65536);

  ok = gTrue;
}

FTFont::~FTFont() {
  gfree(cacheTags);
  gfree(cache);
  gfree(image->data);
  image->data = NULL;
  XDestroyImage(image);
}

GBool FTFont::drawChar(Drawable d, int w, int h, GC gc,
		       int x, int y, int r, int g, int b, Gushort c) {
  FTFontEngine *engine;
  XColor xcolor;
  int bgR, bgG, bgB;
  Gulong colors[5];
  Guchar *p;
  int pix;
  int xOffset, yOffset, x0, y0, x1, y1, gw, gh, w0, h0;
  int xx, yy, xx1;

  engine = fontFile->engine;

  // generate the glyph pixmap
  if (!(p = getGlyphPixmap(c, &xOffset, &yOffset, &gw, &gh))) {
    return gFalse;
  }

  // compute: (x0,y0) = position in destination drawable
  //          (x1,y1) = position in glyph image
  //          (w0,h0) = size of image transfer
  x0 = x - xOffset;
  y0 = y - yOffset;
  x1 = 0;
  y1 = 0;
  w0 = gw;
  h0 = gh;
  if (x0 < 0) {
    x1 = -x0;
    w0 += x0;
    x0 = 0;
  }
  if (x0 + w0 > w) {
    w0 = w - x0;
  }
  if (w0 < 0) {
    return gTrue;
  }
  if (y0 < 0) {
    y1 = -y0;
    h0 += y0;
    y0 = 0;
  }
  if (y0 + h0 > h) {
    h0 = h - y0;
  }
  if (h0 < 0) {
    return gTrue;
  }

  // read the X image
  XGetSubImage(engine->display, d, x0, y0, w0, h0, (1 << engine->depth) - 1,
	       ZPixmap, image, x1, y1);

  if (engine->aa) {

    // compute the colors
    xcolor.pixel = XGetPixel(image, x1, y1);
    XQueryColor(engine->display, engine->colormap, &xcolor);
    bgR = xcolor.red;
    bgG = xcolor.green;
    bgB = xcolor.blue;
    colors[1] = engine->findColor((r + 3*bgR) / 4,
				  (g + 3*bgG) / 4,
				  (b + 3*bgB) / 4);
    colors[2] = engine->findColor((r + bgR) / 2,
				  (g + bgG) / 2,
				  (b + bgB) / 2);
    colors[3] = engine->findColor((3*r + bgR) / 4,
				  (3*g + bgG) / 4,
				  (3*b + bgB) / 4);
    colors[4] = engine->findColor(r, g, b);

    // stuff the glyph pixmap into the X image
    for (yy = 0; yy < gh; ++yy) {
      for (xx = 0; xx < gw; ++xx) {
	pix = *p++ & 0xff;
	pix = (pix * 4) / 255;
	if (pix > 0) {
	  XPutPixel(image, xx, yy, colors[pix]);
	}
      }
    }

  } else {

    // one color
    colors[1] = engine->findColor(r, g, b);

    // stuff the glyph bitmap into the X image
    for (yy = 0; yy < gh; ++yy) {
      for (xx = 0; xx < gw; xx += 8) {
	pix = *p++;
	for (xx1 = xx; xx1 < xx + 8 && xx1 < gw; ++xx1) {
	  if (pix & 0x80) {
	    XPutPixel(image, xx1, yy, colors[1]);
	  }
	  pix <<= 1;
	}
      }
    }

  }

  // draw the X image
  XPutImage(engine->display, d, gc, image, x1, y1, x0, y0, w0, h0);

  return gTrue;
}

Guchar *FTFont::getGlyphPixmap(Gushort c, int *x, int *y, int *w, int *h) {
  FT_GlyphSlot slot;
  FT_UInt idx;
  int gSize;
  int i, j, k;
  Guchar *ret;

  // check the cache
  i = (c & (cacheSets - 1)) * cacheAssoc;
  for (j = 0; j < cacheAssoc; ++j) {
    if ((cacheTags[i+j].mru & 0x8000) && cacheTags[i+j].code == c) {
      *x = cacheTags[i+j].x;
      *y = cacheTags[i+j].y;
      *w = cacheTags[i+j].w;
      *h = cacheTags[i+j].h;
      for (k = 0; k < cacheAssoc; ++k) {
	if (k != j &&
	    (cacheTags[i+k].mru & 0x7fff) < (cacheTags[i+j].mru & 0x7fff)) {
	  ++cacheTags[i+k].mru;
	}
      }
      cacheTags[i+j].mru = 0x8000;
      return cache + (i+j) * glyphSize;
    }
  }

  // generate the glyph pixmap or bitmap
  fontFile->face->size = sizeObj;
  FT_Set_Transform(fontFile->face, &matrix, NULL);
  slot = fontFile->face->glyph;
#if 1 //~
  if (fontFile->useGlyphMap) {
    if (c < 256) {
      idx = fontFile->glyphMap[c];
    } else {
      idx = 0;
    }
  } else {
    idx = FT_Get_Char_Index(fontFile->face, fontFile->charMapOffset + c);
  }
#else
  idx = FT_Get_Char_Index(fontFile->face, fontFile->charMapOffset + c);
#endif
  if (FT_Load_Glyph(fontFile->face, idx, FT_LOAD_DEFAULT) ||
      FT_Render_Glyph(slot,
		      fontFile->engine->aa ? ft_render_mode_normal :
		                             ft_render_mode_mono)) {
    return gFalse;
  }
  *x = -slot->bitmap_left;
  *y = slot->bitmap_top;
  *w = slot->bitmap.width;
  *h = slot->bitmap.rows;
  if (*w > glyphW || *h > glyphH) {
#if 1 //~
    fprintf(stderr, "Weird FreeType glyph size: %d > %d or %d > %d\n",
	    *w, glyphW, *h, glyphH);
#endif
    return NULL;
  }

  // store glyph pixmap in cache
  ret = NULL;
  for (j = 0; j < cacheAssoc; ++j) {
    if ((cacheTags[i+j].mru & 0x7fff) == cacheAssoc - 1) {
      cacheTags[i+j].mru = 0x8000;
      cacheTags[i+j].code = c;
      cacheTags[i+j].x = *x;
      cacheTags[i+j].y = *y;
      cacheTags[i+j].w = *w;
      cacheTags[i+j].h = *h;
      if (fontFile->engine->aa) {
	gSize = *w * *h;
      } else {
	gSize = ((*w + 7) >> 3) * *h;
      }
      ret = cache + (i+j) * glyphSize;
      memcpy(ret, slot->bitmap.buffer, gSize);
    } else {
      ++cacheTags[i+j].mru;
    }
  }
  return ret;
}

#endif // FREETYPE2
#endif // HAVE_FREETYPE_FREETYPE_H | HAVE_FREETYPE_H
