#include <exec/types.h>
#include <exec/memory.h>
#include <fcntl.h>
#include <stdio.h>

#define MakeID(a,b,c,d)  ((a)<<24 | (b)<<16 | (c) <<8 | (d))
#define ID_FORM MakeID('F','O','R','M')
#define ID_LIST MakeID('L','I','S','T')
#define ID_CAT  MakeID('C','A','T',' ')

#define ID_8SVX  MakeID('8','S','V','X')
#define ID_SPRT  MakeID('S','P','R','T')

#define ID_NAME MakeID('N','A','M','E')
#define ID_AUTH MakeID('A','U','T','H')
#define ID_ANNO MakeID('A','N','N','O')
#define ID_CHAN MakeID('C','H','A','N')
#define ID_VHDR MakeID('V','H','D','R')
#define ID_BODY MakeID('B','O','D','Y')
#define ID_CMAP MakeID('C','M','A','P')
#define ID_HGHT MakeID('H','G','H','T')
#define ID_Copyright MakeID('(','c',')',' ')

#define RIGHT 4
#define LEFT 2
#define STEREO 6

#define isodd(num) ((num) & 1)

typedef LONG Fixed;

#define sCmpNone      0
#define sCmpFibDelta  1

typedef struct 
{
  ULONG oneShotSamps,
  repeatSamps,
  sampsPerHiCycle;
  UWORD sampsPerSec;
  UBYTE ctOctave,
  sCompression;
  Fixed volume;
}   Voice8Header;
