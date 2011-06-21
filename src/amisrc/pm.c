/*** PM20.C (MAIN) - This is the main skeleton of the program ************/
/* PACQ © 1989,92 Aaron Hightower                      From: Apr '89
/*                                                     To  : Jan '92
/*
/*   Authors:
/*
/*   Aaron Hightower        Ron Romero                  /~~~~\
/*   1217 Shirley Way       4608 Shady Lake            | o  o |
/*   Bedford, TX 76022      Fort Worth, TX  76180      |      |
/*   Ph: (817) 267-6001     Ph: (817) 498-4396         |/\/\/\|
/*************************************************************************/

#include <stdio.h>            /* standard input/output library (printf) */
#include <string.h>                                      /* strcpy, etc */
#include <exec/types.h>    /* Amiga specific variable type declarations */
#include <graphics/gfx.h>                               /* Graphics def */
#include <graphics/gfxmacros.h>                              /* SetOPen */
#include <exec/memory.h>            /* Used for allocation of CHIP RAM. */
#include <exec/devices.h>             /* Used for joystick read routine */
#include <intuition/intuition.h>        /* for windows, screens, etc... */
#include <intuition/intuitionbase.h>
#include <libraries/dos.h>
#include <graphics/view.h> /* for View, ViewPort, & ColorMap structures */
#include <exec/exec.h>
#include <graphics/gfxbase.h>         /* needed for Opening gfx library */
#include <graphics/sprite.h>             /* needed for sprite functions */
#include <libraries/arpbase.h>                    /* For file requester */

#define global

/* #include "pmenus.h" - This is now defined in pmenus.c module */
#include "pacdef.h"
#include "pacvar.h"
#include "pacvar2.h"
#include "keydef.h"

char def_title[]="PACQ "VERSION" © 1992 Aaron Hightower",
     titlebuff[80],
     *title=def_title;

extern unsigned char curr_filter;
struct NewScreen custScreen = {
       0,0,                        /* screen XY origin relative to View */
       320,200,                              /* screen width and height */
       3,                         /* screen depth (number of bitplanes) */
       0,1,                                    /* detail and block pens */
       SPRITES,                        /* display modes for this screen */
       CUSTOMSCREEN | SCREENBEHIND /* | SCREENQUIET */,  /* screen type */
       NULL,                          /* pointer to default screen font */
       def_title,                                       /* screen title */
       NULL,                  /* first in list of custom screen gadgets */
       NULL,                      /* pointer to custom BitMap structure */
};

struct FileRequester *fileRequester = NULL;

struct NewWindow custWindow = {
       0,0,640,200,0,1,
       MENUPICK | RAWKEY,
       REPORTMOUSE|ACTIVATE|NOCAREREFRESH|BORDERLESS|BACKDROP,
       NULL,NULL,NULL,NULL,NULL,
       -1,-1,-1,-1,   
       CUSTOMSCREEN,                          
};

short spgot[MAXGHOSTS+1] = {-1,-1,-1,-1,-1,-1,-1,};

USHORT scols0[3] =     {0xF00,0xF80,0xFF0}; /* Pacman: Red,Orange,Yellow */
USHORT scols1[6] =     {0xF00,0xF88,           /* Ghosts: RED, PINK     */
                       0x08F,0xF80,            /* Ghosts: BLUE, STUPID  */
                       0x0A2,0xF0F};           /* Ghosts: FUTURE, FUTURE*/
USHORT scols2[2] =     {0x00F,0xAAF};          /* Energized: blink=0,1  */

short gint[MAXGHOSTS] = {RED,BLUE,PINK,STUPID,STUPID,STUPID,};
short speed[MAXGHOSTS] = {1,  1,   1,    1,     1,     1,   };
USHORT speedlist[NUMSPEEDS+1] =
        {SPEED1 ,SPEED2 ,SPEED3 ,SPEED4 ,SPEED5 ,SPEED6 ,SPEED7 ,SPEED8 ,
         SPEED9 ,SPEED10,SPEED11,SPEED12,SPEED13,SPEED14,SPEED15,SPEED16,
         SPEED17};

char sprnums[2][MAXGHOSTS+1] = { {6,5,4,3,2,1,7}, /* Non superbitmap */
                                 {1,2,3,4,5,6,7} }; /* SuperBitMap */

/* Sprites 7 and 6 are do not work correctly on Amiga 3000 */
/* This is due to the position of the display - use preferences to fix */
/* Works fine on Amiga 1000 */
/* Sprite 6 blinks on the superbitmap mazes.  Sprite 7 doesn't appear. */
/* I am making two lists.  One for non-superbitmap, one for superbitmap. */
/* Use the "super" variable in the first bracket of the sprnums variable */
/* The above are the hardware sprite numbers to use in the following order: */
/* PACMAN, GHOST [1..MAXGHOSTS+1] */

void main()
{
    char gamename[50];

    ULONG signalmask, signals;
    UBYTE done;

    OpenAll();

    InitData();

    v_head=NULL;

   restart_program:

    /* At this point we have everything necessary open and ready.. */

    killallsound();

    strcpy(titlebuff,title);

    ChangeScreenWindow(320,200,0);

    SetWindowTitles(Window,titlebuff,titlebuff);

    ClosePlayScreen();
    CloseStatScreen();

    done = 0;
    while(!done)
    {
       signalmask = 1L << Window->UserPort->mp_SigBit;

       signals = Wait(signalmask);
       if(signals & signalmask) done = handleIDCMP(Window);
       /* Done should turn hi when the name of the game is given. */

       WaitTOF();
    }

    strcpy(fr_path,fileRequester->fr_Dir);
    strcpy(fr_name,fileRequester->fr_File);

    strcpy(gamename,fr_path);

    if ( !strchr("/:\0",fr_path[strlen(fr_path)-1]) && strlen(fr_path))
       strcat(gamename,"/");

    if (strlen(gamename) < 2 || !strlen(fr_name))
    {
      title="No file specified!";
      goto restart_program;
    }

    strcat(gamename,fr_name);

    if (ReadGame(gamename))
    { 
      title="Game file not found!";
      goto restart_program;
    }

    score = 0;

    lives = game.numlives;
    extra = game.extraman;
    
   loadmaze:

    killallsound();

    EraseSprites();

    NextMaze();

    rastport = rastPort;
    statport = statRastPort;
    svp = viewPort;

    CloseScreenWindow();

    if (DrawMaze(DOTS_INCLUDED) < 0) lives = -1; /* Maze error: quit */

    speedindex = 0;
    advspeed[0]=(short)((numdots*3) /4 ); /* At these numdots, adv speed */
    advspeed[1]=(short) (numdots    /2 );
    advspeed[2]=(short) (numdots    /4 );
    advspeed[3]=(short) (numdots    /6 );
    advspeed[4]=(short) (numdots    /8 );
    advspeed[5]=(short) (numdots    /15);
    advspeed[6]=(short) (numdots    /20);
    advspeed[7]=         -1;
    
    while(lives>0 && numdots)  /* New pacman or new screen start here */
    {
       PlayMaze();
       if(lives)
         BlinkMaze();
    }
    /* restore the system to its original state */
    if(lives>0) goto loadmaze;
    else if(!lives) sprintf(statline,"Score: %d Game: %s",score,fr_name);
    else sprintf(statline,"Error: Maze too large.");

    title=statline;

    goto restart_program; /* What bad programming, huh? */

} /* Main */

InitData()
{
    fileRequester = ArpAllocFreq();

    seed=7;                 /* Should always be an odd positive integer */

    strcpy(titlebuff,def_title);
    fileRequester->fr_Hail=titlebuff;
    fileRequester->fr_LeftEdge= 0;
    fileRequester->fr_TopEdge = 1;

    boxheight = pathsize = xfactor = yfactor = 2;
    /* Prevent a divide by zero error and ghost box from being fouled up */

    curr_filter = FilterOff(1);                /* Filter defaults as off */

    dheight=MAX(190,GfxBase->NormalDisplayRows-STAT_HEIGHT-2);
    dwidth=320;

    return 0;
}

Die()
{
 short i;

 killallsound();
 BlinkEnergizer(1);     /* Sometimes he thinks he ate the energizer :-) */
                             /* Show the guy the energizer just in case */

 /* Show how PACMAN was killed */
 Delay(60);                                               /* One second */

 /* Make the only the ghosts disappear during death sequence. */
 for(i=0;i<numghosts;i++)
 {
   stay[i]=stayaftdie[i];
   if(wb2_0) MoveSprite(svp,&sprite[i+1],WIDTH<<1,dheight<<1);
   else MoveSprite(NULL,&sprite[i+1],WIDTH<<1,dheight<<1);
 }

 playsound(SND_DIE,1);
 playsound(SND_DIE,1);                /* "STEREO REQUIRES TWO REQUESTS" */

 i=0;
 while(soundplaying(SND_DIE))
 {
  ChangeSprite(svp,&sprite[0],(short *)&pacmen[(1+3*i)%12]);
  Delay(5);
  i++;
 }

 Delay(60);                                               /* One second */

 return 0;
}

InitializeStarting()
{
   short i,k;
   vertex_pt g_strt=ghost_start;

   /*if (!((mazestatus&GRAPH_LOADED)&&(mazestatus&VARS_LOADED))) return 0;*/

   half=0;                               /* Initialize timer variable */
   dead=0;                                         /* PacMan is born! */
   revover=0;
   pdest_v=pac_start1;
   pmv=pac_start2;
   if (!(pac_start1 && pac_start2))
   {
      pdest_v=v_head;
      for(i=0;i<NUMEDGES;i++)
         if (pac_start1->next[i]) pmv=pdest_v->next[i];
   }
   /* Setup PAC-MAN @ start edge with default directions */
   x_dir= (pmv->x != pdest_v->x) * ((pmv->x > pdest_v->x)?-1:1);
   y_dir= (pmv->y != pdest_v->y) * ((pmv->y > pdest_v->y)?-1:1);
   i = joy_dir = (x_dir==0)*(1+y_dir)+(x_dir!=0)*(2-x_dir);
   sprite[0].height=21;
   xposn[0]=(pmv->x+pdest_v->x)/2-XOFF;
   yposn[0]=(pmv->y+pdest_v->y)/2-YOFF;
   ChangeSprite(svp,&sprite[0],(short *)&pacmen[i*3+1]);

   numeyes = num_energized = 0;
   if (!g_strt) g_strt = v_head;
   for(i=0;i<numghosts;i++)            /* Initialize beginning vertex */
   {
      colorvalue[17+((sprnums[super][i+1])<<1)][R_RGB] = (scols1[gint[i]]&0xF00)>>8 ;
      colorvalue[17+((sprnums[super][i+1])<<1)][G_RGB] = (scols1[gint[i]]&0x0F0)>>4;
      colorvalue[17+((sprnums[super][i+1])<<1)][B_RGB] = (scols1[gint[i]]&0x00F);
 
      colorvalue[18+((sprnums[super][i+1]>>1)<<2)][R_RGB] = 0xF; /* White */
      colorvalue[18+((sprnums[super][i+1]>>1)<<2)][G_RGB] = 0xF; /* Ghost */
      colorvalue[18+((sprnums[super][i+1]>>1)<<2)][B_RGB] = 0xF; /* Eyes  */
 
      k=i&3;
      wasintunnel[i]=0;
      inbox[i] = 1;
      if(stay[i]) changespeed(i,boxspeed);
      else changespeed(i,basespeed[i]);
      c=(i+2)&3;
      if(g_strt->next[c]) gv[i]=g_strt->next[c];
      else if(g_strt->next[k]->next[k])
      {
        gv[i]=g_strt->next[k]->next[k];
        inbox[i]=0;
        stay[i]=0; /* Starts out of the box, and cannot stay */
        changespeed(i,basespeed[i]);
      }
      else gv[i]=g_strt;
 
      xposn[i+1]=gv[i]->x-XOFF;                         /* Sprite specs */
      yposn[i+1]=gv[i]->y-YOFF;
 
      sprite[i+1].height=21;
 
      for(c=0;c<NUMEDGES;c++) if(gv[i]->next[c] 
                  && gv[i]->next[c]!=top_box && gv[i]->next[c]!=center_box
                  && (gv[i]->next[c]!=ghost_start || gv[i]==center_box))
      {                                                  /* Travel info */
         gdest_v[i]=gv[i]->next[c];
         gx[i]=(c==1)-(c==3);
         gy[i]=(c==2)-(c==0);
         ChangeSprite(svp,&sprite[i+1],(short *)&ghosts[i*6+c]);
      }
      if(!gdest_v[i]) gdest_v[i]=gv[i];
   }

   k=((spgot[0]&6)<<1);  /* 0..7 becomes 0,0,4,4,8,8,12,12 */
   for(i=0;i<4;i++) /* PACMAN colors - will erase one from above */
   {
      colorvalue[k+17+i][R_RGB]=(scols0[i]&0xF00)>>8;
      colorvalue[k+17+i][G_RGB]=(scols0[i]&0x0F0)>>4;
      colorvalue[k+17+i][B_RGB]=(scols0[i]&0x00F)>>0;
   }

   for(i=0;i<numghosts;i++)
      NormGhostColor(i);                                /* Ghost colors */

   return 0;
}
