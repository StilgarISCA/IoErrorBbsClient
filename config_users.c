/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles editing and displaying the friend and enemy lists from
 * the client configuration menu.
 */
#include "defs.h"
#include "ext.h"

static void printThemedFriendListEntry( const friend *ptrFriend )
{
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.postFriendName );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "%-20s ", ptrFriend->name );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.postFriendText );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "%s\r\n", ptrFriend->info );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.text );
      stdPrintf( "%s", aryAnsiSequence );
      return;
   }

   stdPrintf( "%-20s %s\r\n", ptrFriend->name, ptrFriend->info );
}

/*
 * Does the editing of the friend and enemy lists.
 */
void editUsers( slist *list, int ( *findfn )( const void *, const void * ), const char *name )
{
   register int inputChar;
   register int itemIndex = 0;
   unsigned int invalid = 0;
   int lines;
   char *ptrUserName;
   char aryInfo[50];
   char *ptrEnemyName;
   friend *ptrFriend;

   while ( true )
   {
      /* Build menu */
      if ( !strncmp( name, "enemy", 5 ) )
      {
         printThemedMnemonicText( "\r\n<A>dd  <D>elete  <L>ist  <O>ptions  <Q>uit", color.number );
      }
      else
      {
         printThemedMnemonicText( "\r\n<A>dd  <D>elete  <E>dit  <L>ist  <Q>uit", color.number );
      }
      {
         char aryDisplayLine[80];

         snprintf( aryDisplayLine, sizeof( aryDisplayLine ), "\r\n%c%s list -> ", toupper( name[0] ), name + 1 );
         printThemedMnemonicText( aryDisplayLine, color.forum );
      }
      printAnsiForegroundColorValue( color.text );

      inputChar = inKey();
      switch ( inputChar )
      {
         case 'a':
            {
               bool shouldSkipAdd;

               stdPrintf( "Add\r\n" );
               stdPrintf( "\r\nUser to add to your %s list -> ", name );
               ptrUserName = getName( -999 );
               shouldSkipAdd = false;
               if ( *ptrUserName )
               {
                  if ( slistFind( list, ptrUserName, findfn ) != -1 )
                  {
                     stdPrintf( "\r\n%s is already on your %s list.\r\n", ptrUserName, name );
                     shouldSkipAdd = true;
                  }
                  if ( shouldSkipAdd )
                  {
                     break;
                  }
                  if ( !strcmp( name, "friend" ) )
                  {
                     if ( !( ptrFriend = (friend *)calloc( 1, sizeof( friend ) ) ) )
                     {
                        fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
                     }
                     snprintf( ptrFriend->name, sizeof( ptrFriend->name ), "%s", ptrUserName );
                     stdPrintf( "Enter info for %s: ", ptrUserName );
                     getString( 48, aryInfo, -999 );
                     snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", ( *aryInfo ) ? aryInfo : "(None)" );
                     ptrFriend->magic = 0x3231;
                     if ( !slistAddItem( list, ptrFriend, 0 ) )
                     {
                        fatalExit( "Can't add 'friend'!\n", "Fatal error" );
                     }
                  }
                  else
                  { /* enemy list */
                     ptrEnemyName = (char *)calloc( 1, strlen( ptrUserName ) + 1 );
                     if ( !ptrEnemyName )
                     {
                        fatalExit( "Out of memory adding 'enemy'!\r\n", "Fatal error" );
                     }
                     else
                     {
                        snprintf( ptrEnemyName, strlen( ptrUserName ) + 1, "%s", ptrUserName );
                     }
                     if ( !slistAddItem( list, ptrEnemyName, 0 ) )
                     {
                        fatalExit( "Can't add 'enemy'!\r\n", "Fatal error" );
                     }
                  }
                  stdPrintf( "\r\n%s was added to your %s list.\r\n", ptrUserName, name );
               }
               break;
            }

         case 'd':
            stdPrintf( "Delete\r\n\nUser to delete from your %s list -> ", name );
            ptrUserName = getName( -999 );
            if ( *ptrUserName )
            {
               itemIndex = slistFind( list, ptrUserName, findfn );
               if ( itemIndex != -1 )
               {
                  free( list->items[itemIndex] );
                  if ( !slistRemoveItem( list, itemIndex ) )
                  {
                     fatalExit( "Can't remove user!\r\n", "Fatal error" );
                  }
                  stdPrintf( "\r\n%s was deleted from your %s list.\r\n", ptrUserName, name );
               }
               else
               {
                  stdPrintf( "\r\n%s is not in your %s list.\r\n", ptrUserName, name );
               }
            }
            break;

         case 'e':
            if ( !strncmp( name, "friend", 6 ) )
            {
               stdPrintf( "Edit\r\nName of user to edit: " );
               ptrUserName = getName( -999 );
               if ( *ptrUserName )
               {
                  if ( ( itemIndex = slistFind( list, ptrUserName, findfn ) ) != -1 )
                  {
                     ptrFriend = list->items[itemIndex];
                     if ( !strcmp( ptrFriend->name, ptrUserName ) )
                     {
                        stdPrintf( "Current info: %s\r\n", ptrFriend->info );
                        stdPrintf( "Return to leave unchanged, NONE to erase.\r\n" );
                        stdPrintf( "Enter new info: " );
                        getString( 48, aryInfo, -999 );
                        if ( *aryInfo )
                        {
                           if ( !strcmp( aryInfo, "NONE" ) )
                           {
                              snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "(None)" );
                           }
                           else
                           {
                              snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", aryInfo );
                           }
                        }
                     }
                  }
                  else
                  {
                     stdPrintf( "\r\n%s is not in your %s list.\r\n", ptrUserName, name );
                  }
               }
               break;
            }
            else
            {
               handleInvalidInput( &invalid );
               continue;
            }

         case 'l':
            stdPrintf( "List\r\n\n" );
            if ( !strcmp( name, "friend" ) )
            {
               lines = 1;
               for ( itemIndex = 0; itemIndex < (int)list->nitems; itemIndex++ )
               {
                  ptrFriend = list->items[itemIndex];
                  printThemedFriendListEntry( ptrFriend );
                  lines++;
                  if ( lines == rows - 1 && more( &lines, -1 ) < 0 )
                  {
                     break;
                  }
               }
            }
            else
            {
               lines = 1;
               for ( itemIndex = 0; itemIndex < (int)list->nitems; itemIndex++ )
               {
                  stdPrintf( "%-19s%s", list->items[itemIndex], ( itemIndex % 4 ) == 3 ? "\r\n" : " " );
                  if ( ( itemIndex % 4 ) == 3 )
                  {
                     lines++;
                  }
                  if ( lines == rows - 1 && more( &lines, -1 ) < 0 )
                  {
                     break;
                  }
               }
               if ( itemIndex % 4 )
               {
                  stdPrintf( "\r\n" );
               }
            }
            break;

         case 'q':
         case '\n':
         case ' ':
            stdPrintf( "Quit\r\n" );
            return;

         case 'o':
            if ( !strncmp( name, "enemy", 5 ) )
            {
               stdPrintf( "Options\r\n\nNotify when an enemy's post is killed? (%s) -> ",
                          flagsConfiguration.shouldSquelchPost ? "No" : "Yes" );
               flagsConfiguration.shouldSquelchPost = !yesNoDefault( !flagsConfiguration.shouldSquelchPost );
               stdPrintf( "Notify when an enemy's eXpress message is killed? (%s) -> ",
                          flagsConfiguration.shouldSquelchExpress ? "No" : "Yes" );
               flagsConfiguration.shouldSquelchExpress = !yesNoDefault( !flagsConfiguration.shouldSquelchExpress );
            }
            /* Fall through */

         default:
            handleInvalidInput( &invalid );
            continue;
      }
      invalid = 0;
   }
}
