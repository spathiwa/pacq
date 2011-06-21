/***  PACGFX.C  ***/
#include <stdio.h>              /* standard input/output library (printf)  */
#include <exec/types.h>  /*  Amiga  specific  variable  type declarations  */
#include <graphics/gfxmacros.h>
#include <intuition/intuitionbase.h>
#include <libraries/arpbase.h>
#include <graphics/sprite.h>

#define global extern

#include "pacdef.h"
#include "pacvar.h"
#include "pcon.h"

BoxScreen()
{
 short bx1,by1,bx2,by2,p=pathsize+BORDERSIZE;
 bx1=minx-p;bx2=maxx+p;
 by1=miny-p;by2=maxy+p;
                               /* These routines removed for SupBitMap */
 bx1=(bx1<0)?0:bx1;            /* bx2=(bx2>maxx)?maxx:bx2; */
 by1=(by1<FIRSTY)?FIRSTY:by1;  /* by2=(by2>maxy)?maxy:by2; */

 SetAPen(rastport,FILLCOLOR);
 SetOPen(rastport,OUTLINECOLOR);
 RectFill(rastport,bx1,by1,bx2,by2);
 return 0;
}

PrintStatus()
{
  sprintf(statline,"%-36sLIVES: %02d%20s1UP: ",maze_dat,lives," ");

  scoreoff = (int) TextLength(statport,statline,strlen(statline));

  sprintf(statline,"%-36sLIVES: %02d%20s1UP: %7d",maze_dat,lives," ",score);

  SetAPen(statport,1);

  Move(statport,1,9);

  Text(statport,statline,strlen(statline));

  return 0;
}

PrintScore()
{
  sprintf(statline,"%7d",score);

  SetAPen(statport,1);

  Move(statport,scoreoff+1,9);

  Text(statport,statline,strlen(statline));

  return 0;
}

MazeDraw1(vertex_pt v0)                /* Draw borders or clear pathways */
{
   /* mazedraw = 1: Draw borders.  0 = erase pathways */
   
   if(!v0) return 0;

   MazeDraw1(v0->left);

   for(i=0;i<4;i++)
   {
      xr=(i==1)-(i==3);yr=(i==2)-(i==0);
      j=pathsize*xr,k=pathsize*yr;

      if ( v0->next[i] )
      {
       if(mazedraw) {path=pathsize;inc=pathsize<<1;}
       else         {path=pathsize-1;inc=1;}
  
       if (v0->x          <=maxx && v0->y          <=maxy
        && v0->next[i]->x <=maxx && v0->next[i]->y <=maxy)
       if (mazedraw) for(c=-path;c<=path;c+=inc)
       {
        Move(rastport,v0->x+yr*c+j,v0->y+xr*c+yr*pathsize);
        if(!(v0->code[i]&TUNNEL))
           Draw(rastport,v0->next[i]->x+yr*c-j,
                         v0->next[i]->y+xr*c-yr*pathsize);
        else                                 /* Special case for tunnels */
        {
           tx=(maxx+border)*(xr>0)+(minx-border)*(xr<0)+(v0->x+yr*c)*(!xr);
           ty=(maxy+border)*(yr>0)+(miny-border)*(yr<0)+(v0->y+xr*c)*(!yr);
           Draw(rastport,tx,ty);
        }
       }
       else /* Mazedraw == 0, so erase the edges */
       {
          SetOPen(rastport,PATHCOLOR);
          SetAPen(rastport,PATHCOLOR);
          if((i==1 || i==2) && !(v0->code[i]&TUNNEL))
             RectFill(rastport,v0->x-path,         v0->y-path,
                               v0->next[i]->x+path,v0->next[i]->y+path);
          else if (i==1 || i==2) /* Erase path for tunnel */
          {
             RectFill(rastport,v0->x-path,v0->y-path,
                               xr?border+maxx:v0->x+path,
                               yr?border+maxy:v0->y+path);
    
             RectFill(rastport,xr?minx-border:v0->next[i]->x-path,
                               yr?miny-border:v0->next[i]->y-path,
                               v0->next[i]->x+path,v0->next[i]->y+path);
          }
       }
      }                      /* End of block executed if there is a path */
      else if (mazedraw && !v0->next[i])
      {                                  /* Close up the NULL directions */
         Move(rastport,v0->x+j+k,v0->y+j+k);
         Draw(rastport,v0->x+j-k,v0->y-j+k);
      }
      /* Round the corners (?): */
      if(!mazedraw&& v0->next[i]&&v0->next[(i+1)&3]&& xfactor>4&&yfactor>4)
         WritePixel(rastport,v0->x+(i<2)*pathsize-(i>1)*pathsize
            ,v0->y-(((i+1)&3)<2)*pathsize+(((i+1)&3)>1)*pathsize);
      else if(!mazedraw && !v0->next[i] && !v0->next[(i+1)&3]
         && v0->next[(i+2)&3] && v0->next[(i+3)&3] && xfactor>4&&yfactor>4)
      {
          SetAPen(rastport,FILLCOLOR);
          WritePixel(rastport,v0->x+(i<2)*pathsize-(i>1)*pathsize
             ,v0->y-(((i+1)&3)<2)*pathsize+(((i+1)&3)>1)*pathsize);
          SetAPen(rastport,PATHCOLOR);
      }
   }
   for(i=0;i<4;i++)
   {
      if(!mazedraw && (/*(v0->next[i]==center_box && v0==top_box)
                       ||*/ ((v0->code[i]&GHOST_1WAY) && in_con)
                       || (v0->code[i]&FORBID)))
      {                     /* Make restrictive bar for "No trespassing" */
         xr=(i==1)-(i==3);yr=(i==2)-(i==0);
         j=pathsize*xr,k=pathsize*yr;
         if (!(v0->code[i] & FORBID))
           SetDrPt(rastport,0xAAAA)
         SetAPen(rastport,NOTRESCOLOR);
         Move(rastport,v0->x+j+k,v0->y+j+k);
         Draw(rastport,v0->x+j-k,v0->y-j+k);
         SetAPen(rastport,PATHCOLOR);
         SetDrPt(rastport,0xFFFF);
      }
   }
   MazeDraw1(v0->right);

   return 0;
}

WireFrameDraw(vertex_pt v0)
{
   if(v0->left) WireFrameDraw(v0->left);

   SetAPen(rastport,DOTCOLOR);

   for(i=1;i<3;i++) if (v0->next[i])
   {
      xr=((i==1)-(i==3))<<1;yr=((i==2)-(i==0))<<1;

      if( !(v0->vcode&ENERGIZER) && !(v0->vcode&GHOST_BOX))
      {
         WritePixel(rastport,v0->x,v0->y);
         WritePixel(rastport,v0->next[i]->x,v0->next[i]->y);
      }

      if(!(v0->code[i]&(DOTS|TUNNEL)) && !(v0->vcode&GHOST_BOX))
      {
         Move(rastport,v0->x+xr,v0->y+yr);
         Draw(rastport,v0->next[i]->x-xr,v0->next[i]->y-yr);
      }
      else if ((v0->code[i]&TUNNEL) && !(v0->code[i]&DOTS))
      {
         Move(rastport,v0->x+xr,v0->y+yr);

         if(v0->x > v0->next[i]->x)
         {
            Draw(rastport,maxx+border,v0->y);
            Move(rastport,minx-border,v0->y);
            Draw(rastport,v0->next[i]->x-xr,v0->y);
         }
         else
         {
            Draw(rastport,v0->next[i]->x,maxy+border);
            Move(rastport,v0->x,miny-border);
            Draw(rastport,v0->x,v0->next[i]->y-yr);
         }
      }
   }

   if(v0->right) WireFrameDraw(v0->right);

   return 0;
}


DotDraw2(vertex_pt v0)
{                           /* dvx,dvy,xr,yr are also in other routines */
 if(v0->left) DotDraw2(v0->left);
 ned=0;                            /* Number of edges on this vertex with dots. */
 for(i=0;i<4;i++) if(v0->code[i] & DOTS) ned+=1;
 for(i=0;i<2;i++)
 {
  if((v0->code[i] & DOTS) && v0->next[i])
  {
   xr=(i==1)-(i==3);yr=(i==2)-(i==0);
   dvx=(v0->next[i]->x-v0->x+v0->next[i]->y-v0->y);      /* Sent. value */
   dvy=-1;                             /* Initialize this temp variable */
   for(tx=v0->x,ty=v0->y;;)                       /* Vertex dots later. */
   {
    tx+=xr*xfactor;
    ty+=yr*yfactor;

    if(tx>maxx) tx=minx;
    if(ty>maxy) ty=miny;
    if(tx<minx) tx=maxx;
    if(ty<miny) ty=maxy;

    if(tx==v0->next[i]->x && ty==v0->next[i]->y) break;
    Draw_Dot(0,tx,ty);
   }
  }
 }
 if(ned>0 || (v0->vcode&ENERGIZER)) Draw_Dot(v0->vcode,v0->x,v0->y);
                                   /* Vertices with dot path get a dot */

 if(v0->right) DotDraw2(v0->right);
 return 0;
}

Draw_Dot(short dotcode,short dotx,short doty)
{
 dotread=ReadPixel(rastport,dotx,doty);
 if(dotread!=DOTCOLOR && dotread!=ENERGCOLOR)
 {
  if(dotcode&ENERGIZER)
  {
   SetAPen(rastport,ENERGCOLOR);
   for(tx=-1;tx<2;tx++) for(ty=-1;ty<2;ty++)
    WritePixel(rastport,dotx+tx,doty+ty);                   /* BIG dot! */
   SetAPen(rastport,DOTCOLOR);
  }
  else 
   WritePixel(rastport,dotx,doty);
  numdots++;
 }
 return 0;
}

FlipSpriteY(sprite_type *sp)
{
 UWORD buffer[2];
 int line;
 for(line=1;line<11;line++)
 {
  buffer[0]=sp->packets[line*2];
  buffer[1]=sp->packets[line*2+1];
  sp->packets[line*2]=sp->packets[44-line*2];
  sp->packets[line*2+1]=sp->packets[45-line*2];
  sp->packets[44-line*2]=buffer[0];
  sp->packets[45-line*2]=buffer[1];
 }
 return 0;
}

FlipSpriteX(sprite_type *sp)
{
 UWORD buffer;
 int line,count;
 for(line=2;line<44;line++)
 {
  buffer=sp->packets[line];
  sp->packets[line]=0;
  for(count=0;count<16;count++)
  {
   sp->packets[line]|=(1&buffer);
   sp->packets[line]<<=1;
   buffer>>=1;
  }
 }
 return 0;
}

DrawMaze(short drawdots)
{
   short i;

   DisplaySprites(FALSE);

   SetRast(rastport,0);

#if 0
   if (maxx+border >= width || maxy+border >= height)
      return -1; /* Error: Too big */
#endif

   if (maxx+border >= 1023 || maxy+border >= 1023)
      return -1; /* Error: Too big */

   SetDrMd(rastport,JAM1);

   if (drawdots==DOTS_EXCLUDED) /* Called from construction set */
   {
      if (!((mazestatus&GRAPH_LOADED)&&(mazestatus&VARS_LOADED))) return 0;
      SetRast(rastport,0);
      SetMazeColors();
   }

   BoxScreen();

   SetAPen(rastport,OUTLINECOLOR);
   mazedraw=1;                                /* Draw all of the edges */
   MazeDraw1(v_head);
   MazeDraw1(center_box);
   MazeDraw1(ghost_start);
   for(i=0;i<4;i++) MazeDraw1(box_side[i]);
   MazeDraw1(top_box);

   SetAPen(rastport,PATHCOLOR);
   mazedraw=0;                                   /* Clear the pathways */
   MazeDraw1(v_head);
   MazeDraw1(center_box);
   MazeDraw1(ghost_start);
   for(i=0;i<4;i++) MazeDraw1(box_side[i]);
   MazeDraw1(top_box);

   SetAPen(rastport,DOTCOLOR);
   numdots=0;
   DotDraw2(v_head);

   if(drawdots==DOTS_EXCLUDED)
   {
      WireFrameDraw(v_head);
      if (linedrawn)
      {
         DrawConnection(sv,olddir);
         linedrawn=!linedrawn; /* Nullify the effect within DrawConn... */
      }
   }
   DisplaySprites(sprites_on);

#if MYDEBUG
   printf("View->DxOffset=%d \tScreen->LeftEdge=%d\n",
           curr_View->DxOffset,PlayScreen->LeftEdge);
#endif

   return 1; /* 1 signals main module that maze was drawn successfully */
}
