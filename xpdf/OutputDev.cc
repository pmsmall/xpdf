//========================================================================
//
// OutputDev.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include "OutputDev.h"

void OutputDev::setCTM(double *ctm1) {
  int i;
  double det;

  for (i = 0; i < 6; ++i)
    ctm[i] = ctm1[i];
  det = 1 / (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;
}

void OutputDev::cvtDevToUser(int dx, int dy, double *ux, double *uy) {
  *ux = ictm[0] * dx + ictm[2] * dy + ictm[4];
  *uy = ictm[1] * dx + ictm[3] * dy + ictm[5];
}

void OutputDev::cvtUserToDev(double ux, double uy, int *dx, int *dy) {
  *dx = (int)(ctm[0] * ux + ctm[2] * uy + ctm[4] + 0.5);
  *dy = (int)(ctm[1] * ux + ctm[3] * uy + ctm[5] + 0.5);
}
