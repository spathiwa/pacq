/*** PACVAR.H - These declarations should require few number includes. ***/
/*
/* Global variables for use in most/all modules of the program
/*************************************************************************/

global sprite_type *sprite_data;
global int numsprites; /* Must only be an integer value!
                          This is read from sprite data before data. */
global short sprites_on; /* Used to tell if sprites are on or not. */
global short spgot[MAXGHOSTS+1];

global vertex_pt v_head,temp_pt,gvi; /* Dummy pointer to maze & temp storage */
global vertex_pt ghost_start,pac_start1,pac_start2,top_box,center_box,
       pdest_v,pmv,tv,box_side[4],gbv1,gbv2;
global vertex_pt gdest_v[MAXGHOSTS],gv[MAXGHOSTS];
global vertex_pt *heap,*theap;                 /* Used in ShortTree() */

global struct game game;

global sprite_type *pacmen,*ghosts,*eyes,*scores,*blank;

global short joy_dir,dir_code,old_dcode,x_dir,y_dir;
                                            /* PACMAN movement registers */
global short spoff;                         /* Animation sequence control */
                                            /* Ghosts movedir */
global short gdiri,gdir_code[MAXGHOSTS],gx[MAXGHOSTS],gy[MAXGHOSTS];
global short revtoggle[MAXGHOSTS];   /* Tell ghost to turn around (if he can) */
global short inbox[MAXGHOSTS];
global short num_energized;
global short minx,miny,maxx,maxy;    /* Used to center the maze after read */
global short lookhere;
global BOOLEAN at_vertex;

global USHORT dwidth,dheight,width,height,leftedge;

global struct Screen *Screen;
global struct Window *Window;

global struct View *oldview;
global struct ViewPort *svp;

global struct RastPort *rastport,*statport;
global struct GfxBase *GfxBase;
global struct IntuitionBase *IntuitionBase;

/*****************************************************************/
/* the following global variables are for joystick read routines */
/*****************************************************************/

global struct InputEvent *game_data; /* this is the queue for controller inputs */
global SHORT error;
global struct IOStdReq *game_io_msg;
global BYTE *gamebuff;                     /* Head pointer to gamebuffer */
global SHORT testval;
global SHORT joyx,joyy;
global char button;
global struct MsgPort *game_msg_port;
global SHORT movesize;
extern struct MsgPort *CreatePort(); /* routine to open a port for input */
extern struct IOStdReq *CreateStdIO(); /* routine to request comm. block */
global SHORT codeval,timeouts;

/****************** End of joy-vars ******************************/
/* the following global variables are for general use throughout */
/*****************************************************************/

global short wb2_0,vpdx,vpdy; /* wb2_0==V36 or higher... */
global short dead,killghost;                           /* Is PACMAN dead? */
global short firstrev,revover,minmaxx[MAXGHOSTS],minmaxy[MAXGHOSTS];
global short home[MAXGHOSTS];
global short i,k,j,c,xr,yr,seed;
global int ecode;
global short xposn[MAXGHOSTS+1],yposn[MAXGHOSTS+1]; /* absolute position / objs */
global short xras_dest,yras_dest;
global short superx,supery,super;
global unsigned int score,extra;
global short bonus;
global short lives,half,numeyes;
global short atedot,dotread,ned,numdots,advspeed[8],speedindex; /* dotvars */
global short tx,ty,dvx,dvy;                /* Globals for general purposes */
global short mazedraw,path,inc;               /* Vars for mazedraw routine */
global short offsetx,offsety;   /* Offset computed from min & max of x & y */
global short energized[MAXGHOSTS],stay[MAXGHOSTS],stayaftdie[MAXGHOSTS],
      speed[MAXGHOSTS],movefast[MAXGHOSTS];             /* current speed */
global short basespeed[MAXGHOSTS];                           /* base speed */
global short tunnelspeed,boxspeed,eyespeed,bluespeed,bluetunnel;
global short wasintunnel[MAXGHOSTS]; /* says to reset speed at next vertex */

global short scoreoff;
global char statline[82];

/*USHORT speedlist[NUMSPEEDS];*/          /* list of bit values for speeds */

/****************** End of gen-vars ******************************/

global char sprite_dat[80];            /* file name for the sprite data. */
global char maze_dat[80];              /* file name for the maze   data. */
global char *soundpath;      /* pointer to file for path of sounds files */
global BOOLEAN nosound;

/*********** The following are vars for the data read from maze **********/

global USHORT colortable[32];
global USHORT colorvalue[32][3];
global USHORT mazestatus; /* Is maze loaded/changed, vars loaded/changed */
global char nxt_maze_dat[10];
                                           /* Maze sizing variables */
global short border,pathsize,xfactor,yfactor,boxheight,ghstsproff,
       pacsproff,scrsproff,numghosts,scoff,energtime;

/*************************************************************************/
