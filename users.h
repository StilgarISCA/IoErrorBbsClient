/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

typedef struct
{
   int killposts : 1;      /* Kill posts */
   int killexpress : 1;    /* Kill express messages */
   int notifykills : 1;    /* Notify that a message was killed */
   int displaywholist : 1; /* Display aryUser list on who list */
   int displaylong : 1;    /* Display long (1) or short (0) */
} UserlistAttributes;

typedef struct
{
   char name[41];                 /* Name of aryUser list */
   UserlistAttributes attributes; /* Attributes for aryUser list */
   slist *list;                   /* List of users */
} Userlist;
