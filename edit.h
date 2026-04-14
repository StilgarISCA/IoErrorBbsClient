/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EDIT_H_INCLUDED
#define EDIT_H_INCLUDED

#include "defs.h"

int checkFile( FILE *ptrMessageFile );
int prompt( FILE *ptrMessageFile, int *previousChar, int commandChar );

void makeMessage( int upload );

#endif /* EDIT_H_INCLUDED */
