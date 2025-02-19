//========================================================================
//
// Stream-CCITT.h
//
// Tables for CCITT Fax decoding.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

struct CCITTCode {
  short code;
  short n;
};

struct CCITTCodeTable {
  short numCodes;
  CCITTCode *codes;
};

#define ccittMaxCodeLen 13

#define ccittEOL -2

//------------------------------------------------------------------------
// 2D codes
//------------------------------------------------------------------------

#define twoDimPass   0
#define twoDimHoriz  1
#define twoDimVert0  2
#define twoDimVertR1 3
#define twoDimVertL1 4
#define twoDimVertR2 5
#define twoDimVertL2 6
#define twoDimVertR3 7
#define twoDimVertL3 8

static CCITTCode twoDim1Table[1] = {
  {0x0001, twoDimVert0}
};

static CCITTCode twoDim3Table[3] = {
  {0x0001, twoDimHoriz},
  {0x0002, twoDimVertL1},
  {0x0003, twoDimVertR1}
};

static CCITTCode twoDim4Table[1] = {
  {0x0001, twoDimPass}
};

static CCITTCode twoDim6Table[2] = {
  {0x0002, twoDimVertL2},
  {0x0003, twoDimVertR2}
};

static CCITTCode twoDim7Table[2] = {
  {0x0002, twoDimVertL3},
  {0x0003, twoDimVertR3}
};

static CCITTCode twoDim12Table[1] = {
  {0x0001, ccittEOL}
};

static CCITTCodeTable twoDimTable[ccittMaxCodeLen + 1] = {
  {0, NULL},
  {1, twoDim1Table},
  {0, NULL},
  {3, twoDim3Table},
  {1, twoDim4Table},
  {0, NULL},
  {2, twoDim6Table},
  {2, twoDim7Table},
  {0, NULL},
  {0, NULL},
  {0, NULL},
  {0, NULL},
  {1, twoDim12Table},
  {0, NULL}
};

//------------------------------------------------------------------------
// white run lengths
//------------------------------------------------------------------------

static CCITTCode white4Table[6] = {
  {0x0007, 2},
  {0x0008, 3},
  {0x000b, 4},
  {0x000c, 5},
  {0x000e, 6},
  {0x000f, 7}
};

static CCITTCode white5Table[6] = {
  {0x0007, 10},
  {0x0008, 11},
  {0x0012, 128},
  {0x0013, 8},
  {0x0014, 9},
  {0x001b, 64}
};

static CCITTCode white6Table[9] = {
  {0x0003, 13},
  {0x0007, 1},
  {0x0008, 12},
  {0x0017, 192},
  {0x0018, 1664},
  {0x002a, 16},
  {0x002b, 17},
  {0x0034, 14},
  {0x0035, 15}
};

static CCITTCode white7Table[12] = {
  {0x0003, 22},
  {0x0004, 23},
  {0x0008, 20},
  {0x000c, 19},
  {0x0013, 26},
  {0x0017, 21},
  {0x0018, 28},
  {0x0024, 27},
  {0x0027, 18},
  {0x0028, 24},
  {0x002b, 25},
  {0x0037, 256}
};

static CCITTCode white8Table[42] = {
  {0x0002, 29},
  {0x0003, 30},
  {0x0004, 45},
  {0x0005, 46},
  {0x000a, 47},
  {0x000b, 48},
  {0x0012, 33},
  {0x0013, 34},
  {0x0014, 35},
  {0x0015, 36},
  {0x0016, 37},
  {0x0017, 38},
  {0x001a, 31},
  {0x001b, 32},
  {0x0024, 53},
  {0x0025, 54},
  {0x0028, 39},
  {0x0029, 40},
  {0x002a, 41},
  {0x002b, 42},
  {0x002c, 43},
  {0x002d, 44},
  {0x0032, 61},
  {0x0033, 62},
  {0x0034, 63},
  {0x0035, 0},
  {0x0036, 320},
  {0x0037, 384},
  {0x004a, 59},
  {0x004b, 60},
  {0x0052, 49},
  {0x0053, 50},
  {0x0054, 51},
  {0x0055, 52},
  {0x0058, 55},
  {0x0059, 56},
  {0x005a, 57},
  {0x005b, 58},
  {0x0064, 448},
  {0x0065, 512},
  {0x0067, 640},
  {0x0068, 576}
};

static CCITTCode white9Table[16] = {
  {0x0098, 1472},
  {0x0099, 1536},
  {0x009a, 1600},
  {0x009b, 1728},
  {0x00cc, 704},
  {0x00cd, 768},
  {0x00d2, 832},
  {0x00d3, 896},
  {0x00d4, 960},
  {0x00d5, 1024},
  {0x00d6, 1088},
  {0x00d7, 1152},
  {0x00d8, 1216},
  {0x00d9, 1280},
  {0x00da, 1344},
  {0x00db, 1408}
};

static CCITTCode white11Table[3] = {
  {0x0008, 1792},
  {0x000c, 1856},
  {0x000d, 1920},
};

static CCITTCode white12Table[11] = {
  {0x0001, ccittEOL},
  {0x0012, 1984},
  {0x0013, 2048},
  {0x0014, 2112},
  {0x0015, 2176},
  {0x0016, 2240},
  {0x0017, 2304},
  {0x001c, 2368},
  {0x001d, 2432},
  {0x001e, 2496},
  {0x001f, 2560}
};

static CCITTCodeTable whiteTable[ccittMaxCodeLen + 1] = {
  {0, NULL},
  {0, NULL},
  {0, NULL},
  {0, NULL},
  {6, white4Table},
  {6, white5Table},
  {9, white6Table},
  {12, white7Table},
  {42, white8Table},
  {16, white9Table},
  {0, NULL},
  {3, white11Table},
  {11, white12Table},
  {0, NULL}
};

//------------------------------------------------------------------------
// black run lengths
//------------------------------------------------------------------------

static CCITTCode black2Table[2] = {
  {0x0002, 3},
  {0x0003, 2}
};

static CCITTCode black3Table[2] = {
  {0x0002, 1},
  {0x0003, 4}
};

static CCITTCode black4Table[2] = {
  {0x0002, 6},
  {0x0003, 5}
};

static CCITTCode black5Table[1] = {
  {0x0003, 7}
};

static CCITTCode black6Table[2] = {
  {0x0004, 9},
  {0x0005, 8}
};

static CCITTCode black7Table[3] = {
  {0x0004, 10},
  {0x0005, 11},
  {0x0007, 12}
};

static CCITTCode black8Table[2] = {
  {0x0004, 13},
  {0x0007, 14}
};

static CCITTCode black9Table[1] = {
  {0x0018, 15}
};

static CCITTCode black10Table[5] = {
  {0x0008, 18},
  {0x000f, 64},
  {0x0017, 16},
  {0x0018, 17},
  {0x0037, 0}
};

static CCITTCode black11Table[10] = {
  {0x0008, 1792},
  {0x000c, 1856},
  {0x000d, 1920},
  {0x0017, 24},
  {0x0018, 25},
  {0x0028, 23},
  {0x0037, 22},
  {0x0067, 19},
  {0x0068, 20},
  {0x006c, 21}
};

static CCITTCode black12Table[55] = {
  {0x0001, ccittEOL},
  {0x0012, 1984},
  {0x0013, 2048},
  {0x0014, 2112},
  {0x0015, 2176},
  {0x0016, 2240},
  {0x0017, 2304},
  {0x001c, 2368},
  {0x001d, 2432},
  {0x001e, 2496},
  {0x001f, 2560},
  {0x0024, 52},
  {0x0027, 55},
  {0x0028, 56},
  {0x002b, 59},
  {0x002c, 60},
  {0x0033, 320},
  {0x0034, 384},
  {0x0035, 448},
  {0x0037, 53},
  {0x0038, 54},
  {0x0052, 50},
  {0x0053, 51},
  {0x0054, 44},
  {0x0055, 45},
  {0x0056, 46},
  {0x0057, 47},
  {0x0058, 57},
  {0x0059, 58},
  {0x005a, 61},
  {0x005b, 256},
  {0x0064, 48},
  {0x0065, 49},
  {0x0066, 62},
  {0x0067, 63},
  {0x0068, 30},
  {0x0069, 31},
  {0x006a, 32},
  {0x006b, 33},
  {0x006c, 40},
  {0x006d, 41},
  {0x00c8, 128},
  {0x00c9, 192},
  {0x00ca, 26},
  {0x00cb, 27},
  {0x00cc, 28},
  {0x00cd, 29},
  {0x00d2, 34},
  {0x00d3, 35},
  {0x00d4, 36},
  {0x00d5, 37},
  {0x00d6, 38},
  {0x00d7, 39},
  {0x00da, 42},
  {0x00db, 43}
};

static CCITTCode black13Table[20] = {
  {0x004a, 640},
  {0x004b, 704},
  {0x004c, 768},
  {0x004d, 832},
  {0x0052, 1280},
  {0x0053, 1344},
  {0x0054, 1408},
  {0x0055, 1472},
  {0x005a, 1536},
  {0x005b, 1600},
  {0x0064, 1664},
  {0x0065, 1728},
  {0x006c, 512},
  {0x006d, 576},
  {0x0072, 896},
  {0x0073, 960},
  {0x0074, 1024},
  {0x0075, 1088},
  {0x0076, 1152},
  {0x0077, 1216}
};

static CCITTCodeTable blackTable[ccittMaxCodeLen + 1] = {
  {0, NULL},
  {0, NULL},
  {2, black2Table},
  {2, black3Table},
  {2, black4Table},
  {1, black5Table},
  {2, black6Table},
  {3, black7Table},
  {2, black8Table},
  {1, black9Table},
  {5, black10Table},
  {10, black11Table},
  {55, black12Table},
  {20, black13Table}
};
