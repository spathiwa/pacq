#include <proto/all.h>

/*************************************************************************
 * These are global MACRO definitions.
 *************************************************************************/

#define VERSION         "V0.53"
#define REGISTERED_USER 0

#define MODULUS         65536          /* = 1 << ((BITS FOR INT TYPE)/2) */
#define MULTIPLIER      25173          /* Pseudo random number generator */
#define INCREMENT       13849

#define DEPTH           3
#define STAT_DEPTH      1
#define WIDTH           320
#define STAT_WIDTH      640
#define HEIGHT          190
#define STAT_HEIGHT     12
#define FIRSTY          0        /* First allowed Y position for drawing */
#define MAXGHOSTS       6
#define NUMEDGES        4
#define NUMSCORES       MAXGHOSTS

#define DESIRED_DX      132  /* Desired DxOffset for no sprite problems */
#define COLLDIST        4    /* Closest to ghosts possible without dying */
#define XOFF            8           /* Offset for PACMAN to the vertices */
#define YOFF            10
#define YLEEWAY         31
#define XLEEWAY         31

#define PATH            0x01                      /* codes for vertices */
#define DOTS            0x02           /* These values found in maze.dat */
#define TUNNEL          0x04
#define PACSTART        0x08
#define GHOSTSTART      0x10
#define ENERGIZER       0x20
#define GHOST_1WAY      0x40
#define FORBID          0x80
#define NO_MOD          0x100            /* Dont modify the ecode */
#define GHOST_BOX       0x200          /* Vertex code to mark ghost box */

#define MAKEEDGE        0x01 /* GadgetID's */
#define LOADMAZE        0x02
#define LOADVARS        0x03
#define SAVEMAZE        0x04
#define DRAWMAZE        0x05
#define NEXTSTRIP       0x06
#define QUITMAZED       0x07
#define DELEDGE         0x08
#define AUTODRAW        0x09
#define DISPLAY_SPRITES 0x0A
#define SAVEVARS        0x0B

#define GRAPH_LOADED    1
#define VARS_LOADED     2
#define GRAPH_CHANGED   4
#define VARS_CHANGED    8
#define SIZE_CHANGED    8

#define NOMINMAX        0
#define MINMAX          1

#define SND_DIE         0
#define SND_DOT         1
#define SND_CREDIT      2
#define SND_EXTRA       3
#define SND_FRUIT       4
#define SND_MONSTER     5
#define SND_BACKGRND    6
#define SND_EYES        7
#define SND_ENERGIZE    8

#define BACKCOLOR       0
#define TEXTCOLOR       1
#define PATHCOLOR       2
#define DOTCOLOR        3
#define OUTLINECOLOR    4
#define NOTRESCOLOR     5
#define FILLCOLOR       6
#define ENERGCOLOR      7

#define CTRL_COLORS     0
#define FR_COLORS       1

#define BORDERSIZE      3 /* This is the solid stuff around the board */

#define RED             0 /* intelligence levels 0=highest 3=lowest */
#define PINK            1
#define BLUE            2
#define STUPID          3

#define R_RGB           0
#define G_RGB           1
#define B_RGB           2

#define MAXDIST 0xFFF       /* Big number for initing vertex->curr_dist */

#define INCSCORE(x) \
score+=x; if (extra && score >= extra) lives++,extra=0,playsound(SND_EXTRA,1),PrintStatus()

#define SGN(m) ( ( (m) <0)?-1 : ( (m) >0 )?1:0 )
#define ABS(m) ((m<0)?-(m):m)
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))

/* Ghost speed stuff */
#define SPEED17      0xFFFF      /* 1111 1111 1111 1111 */
#define SPEED16      0xFFFE      /* 1111 1111 1111 1110 */
#define SPEED15      0xFEFE      /* 1111 1110 1111 1110 */
#define SPEED14      0xF7BD      /* 1111 0111 1011 1101 */
#define SPEED13      0xEEEE      /* 1110 1110 1110 1110 */
#define SPEED12      0xDB6D      /* 1101 1011 0110 1101 */
#define SPEED11      0xB5AD      /* 1011 0101 1010 1101 */
#define SPEED10      0xAAD5      /* 1010 1010 1101 0101 */
#define SPEED9       0xAAAA      /* 1010 1010 1010 1010 */
#define SPEED8       0x5525      /* 0101 0101 0010 1010 */
#define SPEED7       0x4A52      /* 0100 1010 0101 0010 */
#define SPEED6       0x2492      /* 0010 0100 1001 0010 */
#define SPEED5       0x1111      /* 0001 0001 0001 0001 */
#define SPEED4       0x0842      /* 0000 1000 0100 0010 */
#define SPEED3       0x0101      /* 0000 0001 0000 0001 */
#define SPEED2       0x0001      /* 0000 0000 0000 0001 */
#define SPEED1       0x0000      /* 0000 0000 0000 0000 */

#define NUMSPEEDS 16             /* Numspeeds should be a power of two. */

/* use: */
/* if (speedlist[speed[i]] & (1 << (half & (NUMSPEEDS-1)))) */

#define SCROLLMAYBE           0
#define SCROLLDEFINITELY      1
#define SCROLLTUNNEL          2

#define STRIP_NEXT            0
#define STRIP_INIT            1

#define DOTS_EXCLUDED         0
#define DOTS_INCLUDED         1

#define REV_TIME              0
#define REV_ENERG             1

#define TOGGLE_TIME           1
#define TOGGLE_ENERG          2

#define ALLBLACK              0
#define SCREENONLY            1

#define TUN_UP                0x01
#define TUN_DOWN              0x02
#define TUN_LEFT              0x04
#define TUN_RIGHT             0x08
#define TUN_UPDOWN            0x10
#define TUN_RIGHTLEFT         0x20

#define CTRLR_RESET           0x00
#define CTRLR_READ            0x01

#define DIR_UP                0x01
#define DIR_DOWN              0x02
#define DIR_LEFT              0x04
#define DIR_RIGHT             0x08

typedef char BOOLEAN;
typedef struct {unsigned short packets[46];} sprite_type;
typedef struct vertex *vertex_pt;

struct vertex
{
       short x,y;                /* position of vertex on the playfield */
       short unitx,unity;

       short curr_dist;         /* This baby is used for min span tree. */
       short backtrack;             /* Which way ghost goes back to box */
       short vcode;     /* What kind of vertex is this (ENERGIZER? etc) */
       short code[4];                   /* DOTS? TUNNEL? PATH(=normal)? */
       vertex_pt next[4];          /* pointer to each connecting vertex */
       vertex_pt left,right;                /* pointers for binary tree */
};

struct maze
{
  char *headername;
  char *graphname;
  struct maze *next;
};

struct game
{
       struct maze *firstmaze,*nextmaze;
       int numlives,extraman;
       char *gamename;
       /* put extra life stuff here... */
};
