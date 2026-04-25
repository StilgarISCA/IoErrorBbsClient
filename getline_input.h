/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GETLINE_INPUT_H_INCLUDED
#define GETLINE_INPUT_H_INCLUDED

#include "defs.h"

void getFiveLines( int which );
char *getName( int quitPriv );
void getString( int length, char *result, int line );
void smartErase( const char *ptrEnd );
int smartName( char *ptrBuffer, char *ptrEnd );
void smartPrint( const char *ptrBuffer, const char *ptrEnd );

#endif // GETLINE_INPUT_H_INCLUDED
