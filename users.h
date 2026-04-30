/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

typedef struct
{
   int displaylong : 1;    // Display long (1) or short (0)
   int displaywholist : 1; // Display aryUser list on who list
   int killexpress : 1;    // Kill express messages
   int killposts : 1;      // Kill posts
   int notifykills : 1;    // Notify that a message was killed
} UserlistAttributes;

typedef struct
{
   UserlistAttributes attributes; // Attributes for aryUser list
   slist *list;                   // List of users
   char name[41];                 // Name of aryUser list
} Userlist;
