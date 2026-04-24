/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILTER_H_INCLUDED
#define FILTER_H_INCLUDED

#include "defs.h"

void continuedDataHelper( void );
void continuedPostHelper( void );
void emitTransformedAnsiSequence( const char *ptrAnsiSequence, size_t sequenceLength,
                                  int isPostContext, int isFriend );
void filterData( int inputChar );
void filterExpress( int inputChar );
void filterPost( int inputChar );
void filterWhoList( int inputChar );
int isAutomaticReply( const char *message );
void morePromptHelper( void );
void notReplyingTransformExpress( char *ptrText, size_t size );
void replyCodeTransformExpress( char *ptrText, size_t size );
void reprintLine( void );

#endif /* FILTER_H_INCLUDED */
