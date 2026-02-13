/*
 * This is the file for I/O defines, different systems do certain things
 * differently, such system-specific stuff should be put here.
 */

#define INPUT_LEFT( __fp ) ( ptrPtyInput - aryPtyInputBuffer < ptyInputLength )
#define ptyget() ( INPUT_LEFT( stdin ) ? *ptrPtyInput++ : ( ( ptyInputLength = read( 0, aryPtyInputBuffer, sizeof aryPtyInputBuffer ) ) < 0 ? -1 : ( ( ptrPtyInput = aryPtyInputBuffer ), *ptrPtyInput++ ) ) )

#define NET_INPUT_LEFT() ( ptrNetInput - aryNetInputBuffer < netInputLength )
#define netget() ( NET_INPUT_LEFT() ? *ptrNetInput++ : ( ( netInputLength = read( net, aryNetInputBuffer, sizeof aryNetInputBuffer ) ) <= 0 ? -1 : ( ( ptrNetInput = aryNetInputBuffer ), *ptrNetInput++ ) ) )
#define netput( __c ) ( putc( __c, netOutputFile ) )
#define netflush() ( fflush( netOutputFile ) )
