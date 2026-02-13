/*
 * This handles the information flowing into the client from the BBS.  It
 * basically understands a very limited subset of the telnet protocol (enough
 * to "fake it" with the BBS) with some extensions to allow the extra features
 * the client provides.  The telnet stuff is unimportant, but it'll confuse the
 * BBS if you alter it.
 *
 * The client tells the BBS it is a client rather a telnet program when it first
 * connects, after that the BBS acts differently, letting the client know when
 * something in particular is being done (entering an X message, posting, etc.)
 * to allow the client to handle that however it wants, as well as providing
 * extra protocol for the start/end of messages/X'aryString and the special who list.
 *
 * This is made more complex by the fact that the client doesn't know it should
 * handle (for example) an X message differently until it receives word from
 * the BBS that an X message should be entered -- but by this time the client
 * may have already sent some of the X message over the network instead of
 * gathering it up locally.  So when the BBS tells the client to go into the
 * local X message mode it also tells the client how many bytes have been
 * passed to it (they count them in the same manner) and throws isAway the excess
 * on its side.  The client has buffered this excess on its side and therefore
 * make it available locally instead of having it lost forever.  Boy, that was
 * a pisser when I realized I'd have to do that!
 */
#include "defs.h"
#include "ext.h"
#include "telnet.h"

int telReceive( int inputByte )
{
   static int state = TS_DATA;               /* Current state of telnet state machine */
   static unsigned char aryTelnetBuffer[80]; /* Generic buffer */
   static int telnetBufferPos = 0;           /* Pointer into generic buffer */
   register int outputIndex;
   char *ptrInputString;

   switch ( state )
   {
      case TS_DATA: /* normal data */
         if ( inputByte == IAC )
         { /* telnet Is A Command (IAC) byte */
            state = TS_IAC;
            break;
         }
         if ( whoListProgress )
         { /* We are currently receiving a whoListProgress */
            filterWhoList( inputByte );
         }
         else if ( isExpressMessageInProgress )
         { /* We are currently receiving an X message */
            filterExpress( inputByte );
         }
         else if ( postProgressState )
         { /* We are currently receiving a post */
            filterPost( inputByte );
         }
         else
         { /* Garden-variety data (I hope!) */
            filterData( inputByte );
         }
         break;

         /* handle various telnet and client-specific IAC commands */
      case TS_IAC:
         switch ( inputByte )
         {

               /*
        * This is sent/received when the BBS thinks the client is
        * 'inactive' as per the normal 10 minute limit in the BBS -- the
        * actual inactive time limit for a client is an hour, this is done
        * to make sure the client is still alive on this end so dead
        * connections can be timed out and not be ghosted for an hour.
        */
            case CLIENT:
#if DEBUG
               stdPrintf( "{IAC CLIENT}" );
#endif
               state = TS_DATA;
               netPutChar( IAC );
               netPutChar( CLIENT );
               break;

            case S_WHO: /* start who list transfer */
#if DEBUG
               stdPrintf( "{IAC S_WHO}" );
#endif
               state = TS_DATA;
               whoListProgress = 1;
               break;

            case G_POST: /* get post */
            case G_FIVE: /* get five lines (X or profile info) */
            case G_NAME: /* get name */
            case G_STR:  /* get string */
            case CONFIG: /* do configuration */
#if DEBUG
               stdPrintf( "{IAC %s",
                          inputByte == G_POST ? "G_POST" : inputByte == G_FIVE ? "G_FIVE"
                                                        : inputByte == G_NAME  ? "G_NAME"
                                                        : inputByte == G_STR   ? "G_STR"
                                                        : inputByte == CONFIG  ? "CONFIG"
                                                                               : "huh?" );
#endif
               state = TS_GET;
               aryTelnetBuffer[telnetBufferPos++] = (unsigned char)inputByte;
               break;

               /*
        * This code is used by the bbs to signal the client to synchronize
        * its count of the current byte we are on.  We then send back a
        * START to the bbs so it can synchronize with us.  NOTE:  If it
        * becomes necessary in the future to introduce incompatible changes
        * in the BBS and the client, this will be how the BBS can
        * differentiate between new clients that understand the new stuff,
        * and old clients which the BBS will probably still want to be
        * compatible with. Instead of START a new code can be sent to let
        * the BBS know we understand the new stuff and will operate
        * accordingly.  If there are multiple different versions of the BBS
        * to worry about as well (gag!) then more logic would be needed, I
        * refuse to worry about this case, if it comes about it'll be after
        * I ever have to worry about or maintain that BBS code!
        */
            case START:
#if DEBUG
               stdPrintf( "{IAC START}" );
#endif
               state = TS_DATA;
               byte = 1;
               netPutChar( IAC );
               netPutChar( START3 );
               break;

            case POST_S: /* Start of post transfer */
#if DEBUG
               stdPrintf( "{IAC POST_S}" );
#endif
               state = TS_DATA;
               postHeaderActive = 1;
               postProgressState = 1;
               filterPost( -1 ); /* tell filter to start working */
               break;

            case POST_E: /* End of post transfer */
#if DEBUG
               stdPrintf( "{IAC POST_E}" );
#endif
               state = TS_DATA;
               postHeaderActive = 0;
               ptrPostBuffer = 0;
               postProgressState = 0;
               isPostJustEnded = 1;
               filterPost( -1 ); /* Tell filter to end working */
               break;

            case MORE_M: /* More prompt marker */
#if DEBUG
               printf( "{IAC MORE_M}" );
#endif
               state = TS_DATA;
               flagsConfiguration.isMorePromptActive ^= 1;
               if ( !flagsConfiguration.isMorePromptActive && flagsConfiguration.useAnsi )
               {
                  morePromptHelper(); /* KLUDGE */
               }
               break;

            case XMSG_S: /* Start of X message transfer */
#if DEBUG
               stdPrintf( "{IAC XMSG_S}" );
#endif
               state = TS_DATA;
               *aryExpressParsing = 0;
               isExpressMessageHeaderActive = 1;
               isExpressMessageInProgress = 1;
               filterExpress( -1 ); /* tell filter to start working */
               break;

            case XMSG_E: /* End of X message transfer */
#if DEBUG
               stdPrintf( "{IAC XMSG_E}" );
#endif
               state = TS_DATA;
               *aryExpressParsing = 0;
               isExpressMessageHeaderActive = 0;
               isExpressMessageInProgress = 0;
               ptrExpressMessageBuffer = aryExpressMessageBuffer;
               filterExpress( -1 ); /* Tell filter to end working */
               if ( shouldSendExpressMessage )
               {
                  sendAnX();
                  shouldSendExpressMessage = 0;
               }
               break;

               /* telnet DO/DONT/WILL/WONT option negotiation commands (ignored) */
            case DO:
            case DONT:
            case WILL:
            case WONT:
#if DEBUG
               stdPrintf( "{IAC %s ",
                          inputByte == DO ? "DO" : inputByte == DONT ? "DONT"
                                                : inputByte == WILL  ? "WILL"
                                                : inputByte == WONT  ? "WONT"
                                                                     : "wtf?" );
#endif
               state = TS_VOID;
               break;

            default:
#if DEBUG
               stdPrintf( "{IAC 0x%2X}", inputByte );
#endif
               state = TS_DATA;
               break;
         }
         break;

         /* Get local mode strings/lines/posts */
      case TS_GET:
         aryTelnetBuffer[telnetBufferPos++] = (unsigned char)inputByte;
         if ( telnetBufferPos == 5 )
         {
            targetByte = byte;
            /* Decode the bbs' idea of what the current byte is */
            bytePosition = ( (long)aryTelnetBuffer[2] << 16 ) + ( (long)aryTelnetBuffer[3] << 8 ) + aryTelnetBuffer[4];
            byte = bytePosition;

            /*
        * If we are more out of sync than our buffer size, we can't
        * recover.  If we are out of sync but not so far out of sync we
        * haven't overrun our buffers, we just go back into our buffer and
        * find out what we erroneously sent over the network, and reuse it.
        */
            if ( byte < targetByte - (int)( sizeof arySavedBytes ) - 1 )
            {
               stdPrintf( "\r\n[Error:  characters lost during transmission]\r\n" );
            }
            state = TS_DATA;
            telnetBufferPos = 0;
            switch ( *aryTelnetBuffer )
            {
               case G_POST: /* get post */
                  if ( flagsConfiguration.isPosting )
                  {
                     flagsConfiguration.shouldCheckExpress = 0;
                     return ( -1 );
                  }
#if DEBUG
                  stdPrintf( "}\r\n" );
#endif
                  makeMessage( aryTelnetBuffer[1] );
                  break;

               case G_FIVE: /* get five lines (X message, profile) */
                  getFiveLines( aryTelnetBuffer[1] );
                  if ( aryTelnetBuffer[1] == 1 && xlandQueue->nobjs > 0 )
                  {
                     sendAnX();
                  }
                  break;

               case G_NAME: /* get name */
                  sendBlock();
                  ptrInputString = getName( aryTelnetBuffer[1] );
                  for ( outputIndex = 0; ptrInputString[outputIndex]; outputIndex++ )
                  {
                     netPutChar( ptrInputString[outputIndex] );
                  }
                  if ( *ptrInputString != CTRL_D )
                  {
                     netPutChar( '\n' );
                     byte += outputIndex + 1;
                  }
                  else
                  {
                     byte++;
                  }
                  break;

               case G_STR: /* get string */
#if DEBUG
                  stdPrintf( " 0x%X} ", aryTelnetBuffer[1] );
#endif
                  sendBlock();
                  getString( aryTelnetBuffer[1], (char *)aryTelnetBuffer, -1 );
                  for ( outputIndex = 0; aryTelnetBuffer[outputIndex]; outputIndex++ )
                  {
                     netPutChar( aryTelnetBuffer[outputIndex] );
                  }
                  netPutChar( '\n' );
                  byte += outputIndex + 1;
                  break;

               case CONFIG: /* do configuration */
#if DEBUG
                  stdPrintf( "}" );
#endif
                  sendBlock();
                  configBbsRc();
                  netPutChar( '\n' );
                  byte++;
                  break;
            }
         }
         break;

         /* Ignore next byte (used for ignoring negotations we don't care about) */
      case TS_VOID:
#if DEBUG
         stdPrintf( "0x%X}", inputByte );
#endif
         /*
    * This patch sends IAC WONT in response to a telnet negotiation;
    * this provides compatibility with a standard telnet daemon, e.g.
    * Heinous BBS.  Added by IO ERROR.
    */
         netPutChar( IAC );
         netPutChar( WONT );
         netPutChar( inputByte );
         /* Fall through */
      default:
         state = TS_DATA;
         break;
   }
   return ( 0 );
}

/*
 * Send signal that block of data follows -- this is a signal to the bbs that
 * it should stop ignoring what we send it, since it begins ignoring and
 * throwing isAway everything it receives from the time it sends an IAC G_*
 * command until the time it receives an IAC BLOCK command.
 */
void sendBlock( void )
{
   netPutChar( IAC );
   netPutChar( BLOCK );
}

/*
 * Send a NAWS command to the bbs to tell it what our window size is.
 */
void sendNaws( void )
{
   char aryString[10];
   register int outputIndex;

   if ( oldRows != getWindowSize() )
   {
      /* Old window max was 70 */
      if ( rows > 110 || rows < 10 )
      {
         rows = 24;
      }
      else
      {
         oldRows = rows;
      }
      snprintf( aryString, sizeof( aryString ), "%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, 0, 0, 0, rows, IAC, SE );
      for ( outputIndex = 0; outputIndex < 9; outputIndex++ )
      {
         netPutChar( aryString[outputIndex] );
      }
   }
}

/*
 * Initialize telnet negotations with the bbs -- we don't really do the
 * negotations, we just tell the bbs what it needs to hear, since we don't need
 * to negotiate because we know the correct state to put the terminal in. The
 * BBS (the queue daemon actually) is kludged on its end as well by the IAC
 * CLIENT command.
 */
void telInit( void )
{
   netPutChar( IAC );
   netPutChar( CLIENT2 );
   netPutChar( IAC );
   netPutChar( SB );
   netPutChar( TELOPT_ENVIRON );
   netPutChar( 0 );
   netPutChar( 1 );
   netPutChar( 0 );
   netPuts( "USER" );
   netPutChar( 0 );
   netPuts( aryUser );
   netPutChar( IAC );
   netPutChar( SE );
   sendNaws();
}
