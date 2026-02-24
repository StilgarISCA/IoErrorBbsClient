/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BROWSER_H
#define BROWSER_H

void filterUrl( const char *ptrLine );
void openBrowser( void );
void beginUrlDetectionReport( void );
void emitUrlDetectionReport( void );
void printWithOsc8Links( const char *ptrText );

#endif
