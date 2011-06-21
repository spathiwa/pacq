/*** PSCREEN.C ***/
#include <exec/types.h>     /* Amiga specific variable type declarations */
#include <graphics/view.h>  /* for View, ViewPort, & ColorMap structures */
#include <graphics/sprite.h>             /* needed for sprite functions */
#include <intuition/screens.h>

#define global extern

#include "pacdef.h"
#include "pacvar.h"

global struct Screen *StatScreen,*PlayScreen;
global struct SimpleSprite sprite[MAXGHOSTS+1];
global struct ViewPort *viewPort;
global struct ViewPort *statViewPort;
global struct View *curr_View;
global struct RasInfo *rasInfo;
global USHORT speedlist[MAXGHOSTS];

struct NewScreen custBackDrop = {
       0,0,                        /* screen XY origin relative to View */
       320,200,                              /* screen width and height */
       1,                      /* screen depth (number of bitplanes) */
       0,1,                                    /* detail and block pens */
       SPRITES,                        /* display modes for this screen */
       SCREENBEHIND | CUSTOMSCREEN | SCREENQUIET, /* type */
       NULL,                          /* pointer to default screen font */
       "PACQ"VERSION" © 1989,92 Aaron Hightower",                /* screen title */
       NULL,                  /* first in list of custom screen gadgets */
       NULL,                      /* pointer to custom BitMap structure */
};

PosAllSprite()   /* For use in_con */
{
   if(wb2_0) for(i=0;i<numghosts+1;i++)
   {
      PosSprite(i);
      MoveSprite(svp,&sprite[i],sprite[i].x,sprite[i].y);
   }
   else for(i=0;i<numghosts+1;i++)
   {
      PosSprite(i);
      MoveSprite(NULL,&sprite[i],sprite[i].x,sprite[i].y);
   }

   return 0;
}

PosSprite(int spr)
{
   short tx,ty;

   tx = xposn[spr] - rasInfo->RxOffset;
   ty = yposn[spr] - rasInfo->RyOffset;

   if (tx > -XOFF && tx < WIDTH) sprite[spr].x = tx+vpdx;
   else if ((tx+=width) > -XOFF && tx < WIDTH) ty--,sprite[spr].x=tx+vpdx;
   else sprite[spr].x = WIDTH+1+vpdx;

   if (ty > -YOFF && ty < dheight) sprite[spr].y = ty +vpdy;
   else sprite[spr].x = WIDTH+1+vpdx;

   return 0;
}

ScrollDisplay(int ccode)
{
  static short px,py;
  int x,y;

  x = xposn[0] - (WIDTH  >> 1) + XOFF;
  y = yposn[0] - (dheight>> 1) + YOFF;

  if (ccode)
  {
    if(x<0) x=(superx)?width-x:0;
    if(y<0) y=0;

    y = (y > (height-dheight)+1) ? (height-dheight)+1:y;
    x = (x > width) ? (width)  :x;

    if (rasInfo->RxOffset!=(x/*&~15*/) || rasInfo->RyOffset!=y ||
                               ccode==SCROLLDEFINITELY)
    {
        xras_dest = (superx && x> width-WIDTH && ccode==SCROLLDEFINITELY)
                 ?width:0;
       if (superx) px = rasInfo->RxOffset = (x/*&~15*/);
       if (supery) py = rasInfo->RyOffset = y;

       BlackOut(ALLBLACK);
       ScreenToFront(PlayScreen);
       ScreenToFront(StatScreen);
       ScrollVPort(viewPort);
       PosAllSprite();
       LoadView(curr_View);
       FadeIn();
    }
  }
  else if (superx && (x+=xras_dest)!= px)
  {
    if (px < 0) /* Wrap around */
    {
      xras_dest=width;
      x+=width;
      px+=width;
    }
    else if (px >= width)
    {
      xras_dest=0;
      x-=width;
      px-=width;
    }
    if(ABS(x - px) > XLEEWAY &&
       speedlist[MIN((ABS(x - px)>>3),15)+1]
       & (1 << (half & (NUMSPEEDS-1) )))
      px+=(x>px)?1:-1;

      if (supery && y!= rasInfo->RyOffset && y>=-YLEEWAY 
          && y<=(height-dheight+YLEEWAY+1)
         && ABS(y - rasInfo->RyOffset) > YLEEWAY)
      rasInfo->RyOffset+=(y>rasInfo->RyOffset)?1:-1;

    rasInfo->RxOffset=px/*&~15*/;

    MakeVPort(curr_View, viewPort);
    MrgCop(curr_View);
  }
  else if (supery && y!= rasInfo->RyOffset && y>=-YLEEWAY 
            && y<=(height-dheight+YLEEWAY)+1)
  {
    if(ABS(y - rasInfo->RyOffset) > YLEEWAY)
      rasInfo->RyOffset+=(y>rasInfo->RyOffset)?1:-1;

    MakeVPort(curr_View, viewPort);
    MrgCop(curr_View);
  }
  LoadView(curr_View);
  PosAllSprite();

  if (ccode!=SCROLLTUNNEL) WaitTOF();

  return 0;
}

FadeIn()
{
  short scale;

  for(scale=28;scale>1;--scale)
  {
    for(i=0;i<8;i++)                     /* Copy colors for this screen */
     SetRGB4(viewPort,i,(short) (colorvalue[i][R_RGB]<<1)/scale,
                        (short) (colorvalue[i][G_RGB]<<1)/scale,
                        (short) (colorvalue[i][B_RGB]<<1)/scale);
    SetRGB4(statViewPort,0,(short) (colorvalue[0][R_RGB]<<1)/scale,/*Bkgrnd */
                           (short) (colorvalue[0][G_RGB]<<1)/scale,
                           (short) (colorvalue[0][B_RGB]<<1)/scale);

    SetRGB4(statViewPort,1,(short) (colorvalue[8][R_RGB]),
                           (short) (colorvalue[8][G_RGB]),
                           (short) (colorvalue[8][B_RGB]));

    for(i=17;i<32;i++)                       /* Copy colors for sprites */
     SetRGB4(viewPort,i,(short) (colorvalue[i][R_RGB]<<1)/scale,
                        (short) (colorvalue[i][G_RGB]<<1)/scale,
                        (short) (colorvalue[i][B_RGB]<<1)/scale);

    for(i=17;i<32;i++) /* Make colors for sprites invisible if in wb2_0 */
     SetRGB4(statViewPort,i,(short) (colorvalue[(!wb2_0)*i][R_RGB]<<1)/scale,
                            (short) (colorvalue[(!wb2_0)*i][G_RGB]<<1)/scale,
                            (short) (colorvalue[(!wb2_0)*i][B_RGB]<<1)/scale);

    PosAllSprite();

    if(StatScreen->TopEdge <= dheight + 2)
    {
       MoveScreen(StatScreen,0,((2+dheight-StatScreen->TopEdge)>>4));
       MakeVPort(curr_View,viewPort);
       MrgCop(curr_View);
       LoadView(curr_View);
    }

    WaitTOF();
  }
  while(StatScreen->TopEdge <= dheight + 2)
  {
     MoveScreen(StatScreen,0,((2+dheight-StatScreen->TopEdge)>>4)+1);
     WaitTOF();
  }

  return 0;
}

SetMazeColors()
{
   WaitTOF();

   LoadRGB4(svp,colortable,8);

   for(i=20;i<32;i++)            /* Copy colors for sprites */
    SetRGB4(svp,i,(short) (colorvalue[i][R_RGB]),
                  (short) (colorvalue[i][G_RGB]),
                  (short) (colorvalue[i][B_RGB]));

   if(!wb2_0) for(i=20;i<32;i++)
    SetRGB4(statViewPort,i,(short) (colorvalue[i][R_RGB]),
                           (short) (colorvalue[i][G_RGB]),
                           (short) (colorvalue[i][B_RGB]));
  return 0;
}

BlackOut(int screenonly)
/* Set the entire colorMap to black! */
{
  if (screenonly>1) return 0;

  WaitTOF();

  for(i=0;i<8;i++)
   SetRGB4(viewPort,i,0,0,0);
  if (!screenonly) for(i=20;i<32;i++)
   SetRGB4(viewPort,i,i,i,i);
  SetRGB4(statViewPort,0,0,0,0);
  SetRGB4(statViewPort,1,0,0,0);

  return 0;
}

EraseSprites()
{
   short i;

   for(i=0;i<numghosts+1;i++) /* Get sprites out of the way... */
      ChangeSprite(svp,&sprite[i],(short *)blank);

   return 0;
}
