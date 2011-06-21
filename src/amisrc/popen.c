/*** POPEN.C
   rnd, OpenAll, NextMaze, FreeMemory, and JoyRead / JoySetup           */

#include <exec/types.h>    /* Amiga specific variable type declarations */
#include <hardware/custom.h>
#include <graphics/gfx.h>               /* Defines the BitMap structure */
#include <graphics/rastport.h>         /* Defines the rastport stucture */
#include <exec/memory.h>            /* Used for allocation of CHIP RAM. */
#include <exec/devices.h>             /* Used for joystick read routine */
#include <graphics/view.h> /* for View, ViewPort, & ColorMap structures */
#include <devices/gameport.h>                      /* gameport routines */
#include <devices/inputevent.h>               /* more gameport stuff... */
#include <intuition/intuition.h>        /* for windows, screens, etc... */
#include <intuition/intuitionbase.h>
#include <exec/exec.h>
#include <graphics/gfxbase.h>         /* needed for Opening gfx library */
#include <graphics/gfxmacros.h>
#include <graphics/sprite.h>             /* needed for sprite functions */
#include <libraries/arpbase.h>

#define global extern
#define DUALPLAYFIELD 0

#include "pacdef.h"
#include "pacvar.h"
#include "pacvar2.h"
#include "keydef.h"

global UBYTE left[]  = {CRSR_LEFT , NUM_4 , 0 };
global UBYTE right[] = {CRSR_RIGHT, NUM_6 , 0 };
global UBYTE up[]    = {CRSR_UP   , NUM_8 , 0 };
global UBYTE down[]  = {CRSR_DOWN , NUM_2 , 0 };

USHORT titlescreen[16] = 
{
   0x000, 0xfff, 0x00f, 0x888,
   0x000, 0xfff, 0x888, 0x00f,
   0x000, 0xfff, 0x888, 0x00f,
   0x000, 0xfff, 0x888, 0x00f,
};

struct NewScreen
NewStatScreen =
{
       0,0,                        /* screen XY origin relative to View */
       STAT_WIDTH,STAT_HEIGHT,               /* screen width and height */
       1,                         /* screen depth (number of bitplanes) */
       0,1,                                    /* detail and block pens */
       HIRES | SPRITES,            /* display modes for this screen */
       SCREENBEHIND|CUSTOMSCREEN|SCREENQUIET,            /* screen type */
       NULL,                          /* pointer to default screen font */
       "",                                              /* screen title */
       NULL,                  /* first in list of custom screen gadgets */
       NULL,                      /* pointer to custom BitMap structure */
},
NewPlayScreen =
{
       0,0,
       0,0, /* Width and height defined later */
       3,
       0,1,
       0 | SPRITES,
       SCREENBEHIND | CUSTOMBITMAP | CUSTOMSCREEN | SCREENQUIET,
       NULL,
       "",
       NULL,
       NULL,
};

struct NewWindow
NewStatWindow =
{
       0,0,640,STAT_HEIGHT,0,1,
       RAWKEY,
       RMBTRAP | ACTIVATE | NOCAREREFRESH | BORDERLESS | BACKDROP,
       NULL,NULL,NULL,NULL,NULL,
       -1,-1,-1,-1,   
       CUSTOMSCREEN,                          
},
NewPlayWindow =
{
       0,0,
       0,0, /* Width and height defined later */
       0,1,
       RAWKEY,
       RMBTRAP | ACTIVATE | NOCAREREFRESH | BORDERLESS | BACKDROP,
       NULL,NULL,NULL,NULL,NULL,
       -1,-1,-1,-1,   
       CUSTOMSCREEN,                          
};

char *noscreen = "You have major problems - we can not open a screen!\n\n";

global struct Custom far custom;

global struct Menu *FirstMenu;

global struct NewScreen custScreen;
global struct NewWindow custWindow;

global short gint[MAXGHOSTS];
global short speed[MAXGHOSTS];
global char sprnums[2][MAXGHOSTS+1],*title,def_title[],titlebuff[];

char *defaultpath = "pw:asound/";

char font0[]="pacq.font",
     font1[]="clean.font",
     font2[]="topaz.font"; /* This WILL NOT WORK if you use "Topaz.font" */

struct TextAttr
pacqAttr   = {font0, 8, FS_NORMAL,FPF_DISKFONT|FPF_DESIGNED},
cleanAttr  = {font1, 8, FS_NORMAL,FPF_DISKFONT|FPF_DESIGNED},
topazAttr  = {font2, 8, FS_NORMAL,FPF_ROMFONT|FPF_DESIGNED};

struct TextFont *LoadThisFont(struct TextAttr *);

rnd(int range)
{
 int clock[2];
 
 timer(clock);
 return clock[1]%range;
}

OpenAll()
{
  CreateBasicMaze(); /* Default maze for construction set */

  strcpy(sprite_dat,"pw:sprites/sprite.dat");

  if((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",36))==NULL)
  {
     wb2_0=0;
     if((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0))==NULL) {
         printf("I can't open my graphics library! Oh, NOOOO!!!");
         FreeMemory(); }
  }
  else wb2_0=1; /* Hey!  We're in workbench 2.0! */

  /* Save the active view for quit time */
  oldview = GfxBase->ActiView;

  if((IntuitionBase=(struct IntuitionBase *)
     OpenLibrary("intuition.library",0))==NULL) {
       printf("Could not open the Intuition Library!");
       FreeMemory(); }

  CloseWorkBench();

  if ((DiskfontBase=(struct Library *)
      OpenLibrary("diskfont.library", 33L))==NULL) {
       printf("Could not open the diskfont library!  DUUUUUUUDE!\n");
       FreeMemory(); }

  /* Get best font available */
  if(( !(defaultFont = LoadThisFont(loresAttr=&pacqAttr))))
  if(( !(defaultFont = LoadThisFont(loresAttr=&cleanAttr))))
  if(( !(defaultFont = LoadThisFont(loresAttr=&topazAttr))))
        { printf("Couldn't open any fonts!  Not even ROM fonts?!?!\n\n");
          FreeMemory(); }

  if((ArpBase = (struct ArpBase *) OpenLibrary("arp.library",0))==NULL) {
       printf("Unable to open the arp.library.\nPlease use the arp.library "
       "V1.3 or higher to run this program.\n");
       FreeMemory(); }

/*  if(JoySetup()) { printf("No Joystick, No program...\n\n");
                   FreeMemory(); } */

    if(ReadSprites()) {
       printf("Error while trying to read sprite file \"%s\"\n\n",sprite_dat);
       printf("Aborting program!\n\n");
       FreeMemory(); }

    if((pacmen=(sprite_type *)AllocMem(sizeof(sprite_type)*12,
                                                       MEMF_CHIP|MEMF_CLEAR))==0)
    {  printf("Error while allocting CHIP RAM.\n");
       FreeMemory(); } /* pacmen[12] */

    if((blank=(sprite_type *)AllocMem(sizeof(sprite_type),
                                               MEMF_CHIP|MEMF_CLEAR))==0)
    {  printf("Error while allocting blank sprite.\n");
       FreeMemory(); } /* pacmen[12] */

    if((ghosts=(sprite_type *)AllocMem(sizeof(sprite_type)*6*MAXGHOSTS
                                                       ,MEMF_CHIP))==0)
    {  printf("Error while allocting CHIP RAM.\n");
       FreeMemory(); } /* ghosts[36] */

    if((eyes=(sprite_type *)AllocMem(sizeof(sprite_type)*6*MAXGHOSTS
                                                       ,MEMF_CHIP))==0)
    {  printf("Error while allocting CHIP RAM.\n");
       FreeMemory(); } /* eyes[24] */

    if((scores=(sprite_type *)AllocMem(sizeof(sprite_type)*NUMSCORES
                                                       ,MEMF_CHIP))==0)
    {  printf("Error while allocting CHIP RAM.\n");
       FreeMemory(); } /* scores[6] */

   if (initsound() || loadsound(defaultpath))
   {
    printf("No sound files will be used.\n");
    nosound = TRUE;
   }

   return 0;
}

ChangeScreenWindow(int width,int height,int dmodes)
{
   custScreen.Width       = custWindow.Width = width;
   custScreen.Height      = custWindow.Height= height;
   custScreen.ViewModes   = dmodes;
   custScreen.DefaultTitle= custWindow.Title = titlebuff;

   if(!(Screen && Window))
   {
      CloseScreenWindow();
      if(wb2_0) /* Center the screen */
         custScreen.LeftEdge=
            ((GfxBase->NormalDisplayColumns>>(!(dmodes&HIRES)))-width)>>1;

      OpenScreenWindow();
   }
   ShowTitle(Screen,1);

   LoadRGB4(svp,titlescreen,16);

   ScreenToFront(Screen);

   AdjustMenus(FirstMenu,Window->WScreen->Font);

   SetMenuStrip(Window, FirstMenu);

   return 0;
}

OpenScreenWindow()
{
   custScreen.Font = loresAttr;

   if(!Screen) if((Screen=(struct Screen *)OpenScreen(&custScreen))==0) {
       printf(noscreen);
       FreeMemory(); }

   custWindow.Screen = Screen;

   if(!Window) if((Window=(struct Window *)OpenWindow(&custWindow))==0) {
       printf("You have major problems - we can not open Window!\n\n");
       FreeMemory(); }

   svp = &Screen->ViewPort;
   rastport = &Screen->RastPort;
   SetFont(rastport,defaultFont);

   fileRequester->fr_Window  = Window;

   return 0;
}

CloseScreenWindow()
{
  if (Window)       { if(Window->MenuStrip) ClearMenuStrip(Window);
                      CloseWindow(Window);
                      Window=NULL; }

  if (Screen)       { CloseScreen(Screen);
                      Screen=NULL; }
  return 0;
}

NextMaze()
{
    if(ReadMaze()) {
       printf("Error while reading maze named \"%s\"\n\n",maze_dat);
       printf("Aborting program!\n\n");
       FreeMemory(); }

    SetupSprites();

    ClosePlayScreen();

    MakeBitMaps();

    return 0;
}

SetupSprites()
{
    short i;

    for(i=0;i<6;i++) pacmen[i] = sprite_data[pacsproff+i];
    for(i=6;i<9;i++)
    {
     pacmen[i]=pacmen[i-6];    /* Flip sprites along 'y' for pacman down */
     FlipSpriteY(&pacmen[i]);

     pacmen[i+3]=pacmen[i-3];     /* Compute pacman eating to the right */
     FlipSpriteX(&pacmen[i-3]);
    }

    for(i=0;i<6*MAXGHOSTS;i++)
    {
     ghosts[i]=sprite_data[ghstsproff+(i%6)];
     if((sprnums[super][(int) (i/6)+1])&1)
        for(c=2;c<43;c+=2) /* Mod sprite for new color of ghost */
           ghosts[i].packets[c+1]|=ghosts[i].packets[c];
    }

    for(i=0;i<NUMSCORES;i++)
     scores[i]=sprite_data[scrsproff+i];

    for(i=0;i<6*MAXGHOSTS;i++)
    {
     eyes[i]=sprite_data[ghstsproff+(i&3)];
     for(c=2;c<43;c+=2)
      eyes[i].packets[c]=0;         /* For your eyes only. No cloaking! */
    }

    for(i=1;i<8;i++) FreeSprite(i);
    for(i=numghosts;i>=0;i--)               /* Pacman underneath ghosts */
      if((spgot[i]=GetSprite(&sprite[i],sprnums[super][i]))<0)
       /* pacman=red/org/yel mon1=red;  mon2=pink; mon3=cyan; mon4=org; */
       {  printf("Sprite %d !available - program aborted. ",i);
          FreeMemory(); }

   return 0;
}

MakeBitMaps()
/* This routine makes a bitmap of the specified size, and defined depth */
/* The variable "bitMap" is used to store the BitMap information.       */
{
  short i;
  int flag=1;
  ULONG RasterSize;

  struct BitMap *bm[2] = {&bitMap  ,&bitMap2};
  short bmwidth[2]     = {NULL     ,WIDTH};
  short bmheight[2]    = {NULL     ,NULL };

  bmwidth[0] = width; bmheight[0] = height + 1;
                      bmheight[1] = dheight;

  for(i=0;i<1 && flag;i++) /* Allocate memory for both bitmaps in DPF */
  {
     InitBitMap(bm[i],DEPTH,bmwidth[i],bmheight[i]);

     RasterSize = bm[i]->BytesPerRow * bm[i]->Rows;
     for(j=0;j<DEPTH;j++)
     {
       bm[i]->Planes[j]=
          (PLANEPTR) AllocMem(RasterSize,MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC);
       if(!bm[i]->Planes[j]) { flag = 0; break; }
     }
  }

  if(!flag)
    FreeMemory("Could not allocate a bitplane!\n\n");

  NewStatScreen.Font = loresAttr;

  if(!StatScreen)
     if((StatScreen=(struct Screen *)OpenScreen(&NewStatScreen))==0) {
       printf(noscreen);
       FreeMemory(); }

  NewStatWindow.Screen = StatScreen;

  if(!StatWindow)
     if((StatWindow=(struct Window *)OpenWindow(&NewStatWindow))==0) {
       printf("You have major problems - we can not open StatWindow!\n\n");
       FreeMemory(); }

  NewPlayScreen.CustomBitMap = bm[0];
  NewPlayWindow.Width=NewPlayScreen.Width=bmwidth[1];
  NewPlayWindow.Height=NewPlayScreen.Height=bmheight[1];

  if(!PlayScreen)
     if((PlayScreen=(struct Screen *)OpenScreen(&NewPlayScreen))==0) {
       printf(noscreen);
       FreeMemory(); }

  NewPlayWindow.Screen = PlayScreen;

  if(!PlayWindow)
     if((PlayWindow=(struct Window *)OpenWindow(&NewPlayWindow))==0) {
       printf("You have major problems - we can not open a PlayWindow!\n\n");
       FreeMemory(); }

  ShowTitle(StatScreen,0); /* Hide the title bars */
  ShowTitle(PlayScreen,0);
  SetPointer(PlayWindow,(void *)blank,21,16,0,0);
  SetPointer(StatWindow,(void *)blank,21,16,0,0);

  curr_View = GfxBase->ActiView;         /* Get the view for LoadView()s */

  viewPort=&PlayScreen->ViewPort;     /* Steal the viewport from screens */
  statViewPort=&StatScreen->ViewPort;

  rastPort = &PlayScreen->RastPort;
  statRastPort = &StatScreen->RastPort;

  InitRastPort(&rastPort2);
  SetFont(&rastPort2, defaultFont);
  rastPort2.BitMap = bm[1];

  rasInfo = viewPort->RasInfo;
  statRasInfo = statViewPort->RasInfo;

#if DUALPLAYFIELD /* This is from when I was thinking DUAL PLAYFIELD */
  rasInfo2.BitMap = bm[1];
  rasInfo2.RxOffset = 0;
  rasInfo2.RyOffset = 0;
  rasInfo2.Next = NULL;

  rasInfo->BitMap = bm[0];
  rasInfo->RxOffset = 0;
  rasInfo->RyOffset = 0;
  rasInfo->Next = &rasInfo2;

  Forbid();
  curr_View->Modes |= (DUALPF | PFBA);
  viewPort->Modes  |= (DUALPF | PFBA); */ No longer need this either */
  Permit();
#endif

  Forbid();

  viewPort->DxOffset=DESIRED_DX-curr_View->DxOffset;
  statViewPort->DxOffset=(DESIRED_DX-curr_View->DxOffset)<<1;
  if(!wb2_0) vpdx=viewPort->DxOffset,vpdy=viewPort->DyOffset;

  Permit();

  BlackOut(ALLBLACK);

#if MYDEBUG
  PrintColorMapInfo(PlayScreen);
#endif

  ScreenToFront(PlayScreen);
  ScreenToFront(StatScreen);
  curr_View=GfxBase->ActiView;

  MakeVPort(curr_View,statViewPort);
  MrgCop(curr_View);
  MakeVPort(curr_View,viewPort);
  MrgCop(curr_View);
  LoadView(curr_View);

  return 0;
}

FreeBitMap(struct BitMap *bm,int d)
/* Pass the value of the BitMap pointer to free the bitplanes. */
{
  short i;

  if(bm) for(i=0;i<d;i++)
     if (bm->Planes[i]) {
        FreeMem(bm->Planes[i],bm->BytesPerRow * bm->Rows);
        bm->Planes[i] = NULL;
     }

  return 0;
}

FreeMaze()
{
   DelGhostBox();
   FreeGraph(v_head);
   v_head = pac_start1 = pac_start2 = NULL;
   gbv1=gbv2=NULL; /* Nullify the ghost box edge */
   mazestatus&=~GRAPH_LOADED;

   return 0;
}

FreeGraph(vertex_pt fm)
{
   if(fm)
   {
      FreeGraph(fm->left );
      FreeGraph(fm->right);

      FreeMem(fm,sizeof(struct vertex));
   }
   return 0;
}

ClosePlayScreen()
{
   if(PlayScreen) /* Hopefully this will eliminate flashing problems */
   {
      if (rasInfo)
      {
         EraseSprites();
         Scrollit(-(rasInfo->RxOffset),-(rasInfo->RyOffset));
         MakeVPort(curr_View,viewPort);
         MrgCop(curr_View);
         LoadView(curr_View);
      }

      BlackOut(ALLBLACK);

      if(StatScreen) MoveScreen(StatScreen,0,-(StatScreen->TopEdge));

      ScreenToBack(PlayScreen);
      WaitTOF(); /* ? */

      if(PlayWindow)
      {
         CloseWindow(PlayWindow);
         PlayWindow=NULL;
      }
#if DUALPLAYFIELD
      Forbid();

      curr_View->Modes &= ~(DUALPF | PFBA);
      viewPort->Modes  &= ~(DUALPF | PFBA);
      rasInfo->Next = NULL;
      rasInfo = NULL;

      Permit();
#endif

      CloseScreen(PlayScreen);
      FreeBitMap(&bitMap,DEPTH);

#if DUALPLAYFIELD
      FreeBitMap(&bitMap2,DEPTH);
#endif

      PlayScreen=NULL;
   }
   return 0;
}

CloseStatScreen()
{
    if(StatWindow) {    CloseWindow(StatWindow);
                        StatWindow=NULL; }
    if(StatScreen) {    CloseScreen(StatScreen);
                        StatScreen=NULL; }

    return 0;
}   

/* return user and system-allocated memory to sys-manager */
FreeMemory()
{
    for(i=0;i<MAXGHOSTS+1;i++) if(spgot[i]>-1) FreeSprite(spgot[i]);

    FreeMaze();
    v_head=NULL;

    fileRequester->fr_Window = NULL; /* ??? Who knows...? */

    ClosePlayScreen();
    CloseStatScreen();
    CloseScreenWindow2();
    CloseScreenWindow();

    if (oldview) LoadView(oldview);           /* Put back the old View. */

    if(ArpBase) {      CloseLibrary((void *)ArpBase);
                       ArpBase=NULL; }

    OpenWorkBench();

    if(defaultFont) {  CloseFont(defaultFont);
                       defaultFont=NULL; }

    if(DiskFontBase) { CloseLibrary((void *)DiskFontBase);
                       DiskFontBase=NULL; }

    if(IntuitionBase) { CloseLibrary((void *)IntuitionBase);
                        IntuitionBase=NULL; }

    if(GfxBase) {      CloseLibrary((void *)GfxBase); /* Kill GfxBase */
                       GfxBase=NULL; }

    if(pacmen) {       FreeMem(pacmen,sizeof(sprite_type)*12);
                       pacmen=NULL; }
    if(blank) {                FreeMem(blank,sizeof(sprite_type));
                       blank=NULL; }
    if(ghosts) {       FreeMem(ghosts,sizeof(sprite_type)*6*MAXGHOSTS);
                       ghosts=NULL; }
    if(eyes)   {       FreeMem(eyes,sizeof(sprite_type)*6*MAXGHOSTS);
                       eyes=NULL; }
    if(scores) {       FreeMem(scores,sizeof(sprite_type)*NUMSCORES);
                       scores=NULL; }
    if(sprite_data) {  FreeMem(sprite_data,sizeof(sprite_type)*numsprites);
                       sprite_data=NULL; }
    closesound();                 /* Free memory for the sound routines */

/*    if (game.gamename) FreeMem(game.gamename,strlen(game.gamename)+1); */

/*    if (game.maze)
    {
      FreeMem(game.maze,sizeof(struct maze)*(game.nummazes));

      for (i=0;i<game.nummazes;i++)
      {  
       if (game.maze[i].headername) 
        FreeMem(game.maze[i].headername,strlen(game.maze[i].headername)+1);

       if (game.maze[i].graphname) 
        FreeMem(game.maze[i].graphname,strlen(game.maze[i].graphname)+1);
      }
    } */

    exit(0);

    return 0;
}

JoyRead(short function)       /* returns joyx, joyy, and button */
{
   static char controller=KEYBOARD;
   static UBYTE keydir;

   UWORD joybuf = custom.joy1dat;
   
   if(controller == JOYSTICK || 
      (joybuf & 2) || (joybuf & 512) ||
      ((!(joybuf & 512))^(!(joybuf & 256))) ||
      ((!(joybuf & 2))^(!(joybuf & 1))) )
   {
      controller = JOYSTICK;
      joyx =                            (joybuf & 2)?1:
                                     (joybuf & 512)?-1:0;
      joyy =  ((!(joybuf & 512))^(!(joybuf & 256)))?-1:
                   ((!(joybuf & 2))^(!(joybuf & 1)))?1:0;
      button =      ((*((char *) 0xBFE001)) & 128)?0:1;
   }

   while((message=(struct IntuiMessage *)GetMsg(PlayWindow->UserPort)) ||
         (message=(struct IntuiMessage *)GetMsg(StatWindow->UserPort)))
   {
      code = message->Code;
      /* When we're through with a message, reply */
      ReplyMsg((struct Message *)message);
      /* See what events occurred */
      if(message->Class==RAWKEY)
      {
         char c = code & 127;
         char kd = (c == code);

         if(code==Q_KEY || code==ESC_KEY)
         {
            joyx=joyy=-1;
            dead=-1;
            lives=0;
            break;
         }

              if(strchr(left,c))  c=DIR_LEFT,joyx=-1,joyy=0;
         else if(strchr(up,c))    c=DIR_UP,joyx=0,joyy=-1;
         else if(strchr(right,c)) c=DIR_RIGHT,joyx=1,joyy=0;
         else if(strchr(down,c))  c=DIR_DOWN,joyx=0,joyy=1;
         else continue;

         controller=KEYBOARD;
         if(kd) keydir |= c;
         else
         {
            keydir &= ~c; /* Remove from flags */

            if(keydir & DIR_LEFT) joyx=-1;
            else if(keydir & DIR_RIGHT) joyx=1;
            else joyx=0;
         
            if(keydir & DIR_UP) joyy=-1;
            else if(keydir & DIR_DOWN) joyy=1;
            else joyy=0;
         }
      }
   }

   if(function==CTRLR_RESET) keydir = joyx = joyy = 0;

   return 0;
}

struct TextFont *LoadThisFont( struct TextAttr *TextAttr )
{
  struct TextFont *font;

    if ((font = (struct TextFont *) OpenFont( TextAttr ))
         && font->tf_YSize != TextAttr->ta_YSize)
    {
       CloseFont( font );
       font = 0;
    }
    if(!font)
    {
       TextAttr->ta_Flags = FPF_DISKFONT | FPF_ROMFONT;
       font = (struct TextFont *) OpenDiskFont( TextAttr );
    }

    return font;
}

#if MYDEBUG
PrintColorMapInfo(struct Screen *scn)
{
   struct ColorMap *cm;
   UWORD *table;

   if(scn)
   {
      short i;

      cm=scn->ViewPort.ColorMap;
      if(cm)
      {
         printf("Count: %d \n",cm->Count);
         printf("Flags: %x \n",cm->Flags);
         table=cm->ColorTable;
         for(i=0;i<cm->Count;i++)
         {
            printf("c[%d]=0x%x\t",i,table[i]);
         }
         printf("\n");
      }
      else printf("No colormap available\n");
   }

   return 0;
}
#endif
