/*** PCON.C - This is the module that handles construction of mazes ******/
/*
/*************************************************************************/

#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <libraries/arpbase.h>
#include <graphics/gfxmacros.h> /* SetOPen Macro */
#include <graphics/sprite.h>             /* needed for sprite functions */
#include <graphics/gfxbase.h>

#include "pacdef.h"

#define global
#include "pcon.h"

#undef  global
#define global extern
#include "pacvar.h"
#include "pacvar2.h"
#include "pgadget.h"
#include "keydef.h"

struct NewScreen custScreen2 = {
       0,0, /* GADSCRN_Y, */       /* screen XY origin relative to View */
       640,SCREEN2HEIGHT,                    /* screen width and height */
       2,                         /* screen depth (number of bitplanes) */
       0,1,                                    /* detail and block pens */
       HIRES /* | SPRITES */,          /* display modes for this screen */
       CUSTOMSCREEN | SCREENQUIET | SCREENBEHIND,  /* screen type */
       NULL,                          /* pointer to default screen font */
       "MazEd © 1992 Aaron Hightower & Ron Romero",     /* screen title */
       NULL,                  /* first in list of custom screen gadgets */
       NULL,                      /* pointer to custom BitMap structure */
};

struct NewWindow custWindow2 = {
       0,0,640,SCREEN2HEIGHT,0,1,
       RAWKEY | GADGETDOWN | MOUSEBUTTONS,
       RMBTRAP | ACTIVATE | NOCAREREFRESH | BORDERLESS | BACKDROP,
       NULL,NULL,NULL,NULL,NULL,
       -1,-1,-1,-1,   
       CUSTOMSCREEN,                          
};

Construction()
{
   int breakloop=0;
   ULONG signalmask,signalmask2,signals;

   in_con = 1,linedrawn = 0,olddir = -1,sv=NULL;

   if(!(mazestatus & GRAPH_LOADED)) ReadGraph(basicmaze);
   if(!(mazestatus & VARS_LOADED)) ReadHeader(basicvars);

   width=height=0;     /* This tells ResizeMaze to make the bitmaps */
   ResizeMaze(v_head); /* Needed to perform IsolateGhostBox et al */
                       /* in event that maze loaded is from a playgame */
   CloseScreenWindow();

   OpenScreenWindow2();
   signalmask2 = (1L << Window2->UserPort->mp_SigBit);

   DrawMaze(DOTS_EXCLUDED);
   sprites_on=(gadget[DSgad].Flags & SELECTED);

   do
   {
      /* The following is necessary since the Window may be different */
      signalmask  = (1L << PlayWindow->UserPort->mp_SigBit);

      signals = Wait(signalmask | signalmask2);
      if(signals & signalmask2) breakloop = handleIDCMP2(Window2);
      if(signals & signalmask) breakloop |= handleIDCMP1b(PlayWindow);

   } while(!breakloop) ;

   CloseScreenWindow2();

   ChangeScreenWindow(320,200,0);
  
   ClosePlayScreen();
   CloseStatScreen();

   title=def_title;

   in_con=0;

   return 0;
}

OpenScreenWindow2()
{
   custScreen2.Font = loresAttr;

   if(!Screen2) if(!(Screen2=(struct Screen *)OpenScreen(&custScreen2))) {
       printf("You have major problems - we can not open a screen!\n\n");
       FreeMemory(); }

   ShowTitle(Screen2,0); /* Hide the title bar */
   LoadColors(&(Screen2->ViewPort),CTRL_COLORS);

   svp2 = &(Screen2->ViewPort);
   custWindow2.Screen = Screen2;

   if(!Window2) if(!(Window2=(struct Window *)OpenWindow(&custWindow2))) {
       printf("You have major problems - we can not open Window2!\n\n");
       FreeMemory(); }

   statport = &Screen2->RastPort;
   SetFont(statport,defaultFont);
   ClearPointer(Window2);

   SetupGadgets();

   NextStrip(STRIP_INIT);
   
   OffGadget(&gadget[TUNNELGAD],Window2,NULL);
#if 0 && !REGISTERED_USER /* Disable this - I have a better idea... */
   OffGadget(&gadget[SAVEGRAPHGAD],Window2,NULL);
   OffGadget(&gadget[SAVEVARSGAD],Window2,NULL);
#endif

   ToggleScreenToFront(1);

   return 0;
}

LoadColors(struct ViewPort *vp, short i)
{
   static short colors[][] =
   {
      { 0x888,0x000,0xEEE,0x00E }, /* Control Colors CTRL_COLORS */
      { 0x000,0xEEE,0x000,0x00E }, /* FileRequester Colors FR_COLORS */
   };

   LoadRGB4(vp,colors[i],4);

   return 0;
}

void SetupGadgets()
{
   SHORT currentx=0,i,j,k,l,m;

   for (i=0;i<MAXSTRLEN;i++)
   {
      j=(SHORT) i*2;
      l=j+1;
      k=15+8*i;

      gadgetxy[j][1]=gadgetxy[j][2]=gadgetxy[j][3]=gadgetxy[j][4]=gadgetxy[l][5]=0;
      gadgetxy[j][6]=gadgetxy[j][8]=gadgetxy[j][9]=gadgetxy[l][0]=gadgetxy[l][7]=1;
      gadgetxy[j][7]=gadgetxy[l][9]=9;
      gadgetxy[j][5]=gadgetxy[l][1]=gadgetxy[l][3]=10;
      gadgetxy[j][0]=gadgetxy[l][6]=gadgetxy[l][8]=k;
      gadgetxy[l][2]=gadgetxy[l][4]=k+1;

      for (m=0;m<2;m++)
      {
         gadgetborder[m][j].FrontPen=(m?1:2);
         gadgetborder[m][l].FrontPen=(m?2:1);
         gadgetborder[m][j].DrawMode=gadgetborder[m][l].DrawMode=JAM1;
         gadgetborder[m][j].Count   =gadgetborder[m][l].Count   =5;
         gadgetborder[m][j].XY      =gadgetxy[j];
         gadgetborder[m][l].XY      =gadgetxy[l];
         gadgetborder[m][j].NextBorder=&gadgetborder[m][l];
      }
   }

   for (i=0;i<NUMGADS;i++)
   {
      gadgetintui[i].FrontPen=1; /* Initialize all Intuitexts */
      gadgetintui[i].BackPen=0;
      gadgetintui[i].DrawMode=JAM2;
      gadgetintui[i].LeftEdge=5;
      gadgetintui[i].TopEdge=2;
      gadgetintui[i].ITextFont= loresAttr;
      gadgetintui[i].IText=gadgettxt[i];
      gadgetintui[i].NextText=NULL;

      if(gadID[i]==NEXTSTRIP) currentx = 0;
      gadget[i].LeftEdge = currentx;
      gadget[i].TopEdge = 1;
      gadget[i].Flags = GADGHIMAGE;
      gadget[i].Activation = GADGIMMEDIATE | RELVERIFY;
      if (gadID[i] == AUTODRAW)
      {
         gadget[ADgad=i].Activation |= TOGGLESELECT;
         gadget[i].Flags |= SELECTED;
         autodraw = (gadget[ADgad].Flags & SELECTED);
      }
      else if (gadID[i] == DISPLAY_SPRITES)
      {
         gadget[DSgad=i].Activation |= TOGGLESELECT;
         gadget[i].Flags |= SELECTED;
         sprites_on=((gadget[DSgad].Flags & SELECTED)!=0);
      }
      gadget[i].GadgetType = BOOLGADGET;
      gadget[i].GadgetText = &gadgetintui[i];
      gadget[i].GadgetID = gadID[i];

      gadget[i].GadgetRender =
         (APTR) &gadgetborder[0][(strlen(gadgettxt[i])-1)<<1];
      gadget[i].SelectRender =
         (APTR) &gadgetborder[1][(strlen(gadgettxt[i])-1)<<1];

      gadget[i].Width = (strlen(gadgettxt[i])+1)<<3;
      gadget[i].Height= 11;
      gadget[i].NextGadget =
         (i!=(NUMGADS-1) && gadID[i]!=QUITMAZED)?&gadget[i+1]:NULL;

      currentx += ((strlen(gadgettxt[i])+1)<<3)+2;
   }

   i=0;
   while(!(gadget[i].GadgetID&FLAGGADG)) i++; /* Find first */
   firstflaggad = i;

   for  (;i<=CREATEGAD;i++) 
   {
      gadget[i].Activation |= TOGGLESELECT;
      gadget[i].Flags &= ~(SELECTED);
   }
}


CloseScreenWindow2()
{
  if (Window2)      { CloseWindow(Window2);
                      Window2=NULL; }

  if (Screen2)      { CloseScreen(Screen2);
                      Screen2=NULL; }

  return 0;
}



ResizeMaze(vertex_pt v_head) /* Let xfactor etc do their thing */
{
   short oldwidth, oldheight;

   #if MYDEBUG
   printf("oldwidth: %d oldheight: %d ",width,height);
   #endif
   
   oldwidth=width,oldheight=height;
   maxx=-32000;maxy=-32000;minx=32000;miny=32000;

   ComputeXYs(v_head);
   AdjustMazeVars(MINMAX); /* This changes width, height.. */

   #if MYDEBUG
   printf("width: %d height: %d \n",width,height);
   #endif

   if((oldwidth!=width || oldheight!=height) && in_con) /* For constr only */
   {
      if(rasInfo && (oldwidth || oldheight))
         oldrx=rasInfo->RxOffset,oldry=rasInfo->RyOffset;
      else oldrx=(width-WIDTH)>>1,oldry=(height-dheight)>>1;

      NewPlayWindow.IDCMPFlags|=MOUSEBUTTONS;
      NewPlayWindow.Flags|=REPORTMOUSE;

      ClosePlayScreen();
      MakeBitMaps(); /* Opens/reconfigs the PlayScreen and StatScreen */

      fileRequester->fr_Window=PlayWindow;
      rastport = &PlayScreen->RastPort;
      svp = &PlayScreen->ViewPort;

      Scrollit(oldrx,oldry); /* Put back in approximately the same place */

      NewPlayWindow.IDCMPFlags&=~MOUSEBUTTONS;
      NewPlayWindow.Flags&=~REPORTMOUSE;

      ClearPointer(PlayWindow);
      ClearPointer(StatWindow);
      ScreenToFront(StatScreen);
      if(Screen2) ToggleScreenToFront(1);
      ScreenToFront(PlayScreen);
      curr_View=GfxBase->ActiView; /* We want the view with both
                                        screens in front */
      if(Screen2) ScreenToFront(Screen2);
   }

   MakeGhostBox(gbv1,gbv2);
   IsolateGhostBox();

   return 0;
}

void ComputeXYs(vertex_pt v0) /* Also compute maxx, etc. */
{
   if (!v0) return;
   ComputeXYs(v0->left);
   ComputeXYs(v0->right);

   v0->x = xfactor*(v0->unitx);
   if (v0->x > maxx) maxx = v0->x;
   if (v0->x < minx) minx = v0->x;

   v0->y = yfactor*(v0->unity);
   if (v0->y > maxy) maxy = v0->y;
   if (v0->y < miny) miny = v0->y;
}

vertex_pt AddConnectVert(short x1,short y1, BOOL vertedge)
/* vertedge == 1 when edge is vertical (x1 == x2) */
{
   vertex_pt v0,v2 = NULL, v3=NULL;
   ULONG color;
   short tempxy;

   color = ReadPixel(rastport, x1,y1);
   if (((color == DOTCOLOR) || (color == ENERGCOLOR))
      && (!get_vertex(x1,y1))) /* New Vertex on Existing Edge */
      if (vertedge)
      {
         for (tempxy = x1 + xfactor;
              !(v2 = get_vertex(tempxy,y1)); 
              tempxy += xfactor) /* dont do tunnels yet*/
            ;
         v3 = v2->next[3];
         ecode = v2->code[3]&(~(GHOST_1WAY|FORBID));
      }
      else /* !vertedge */
      {
         for (tempxy = y1 + yfactor;
              !(v2 = get_vertex(x1,tempxy)); 
              tempxy += yfactor) /* dont do tunnels yet*/
            ;
         v3 = v2->next[0];
         ecode = v2->code[0]&(~(GHOST_1WAY|FORBID));
      }
   v0 = add_vertex(x1,y1);
   if (v2)
   {
     connectum(v0,v2);
     connectum(v2,v0);
   }
   if (v3)
   {
     connectum(v0,v3);
     connectum(v3,v0);
   }

   return v0;
}

ToggleScreenToFront(short control)
{
   static status;

   if(control && Screen2) {
      status=0;
      MoveScreen(Screen2,0,dheight-SCREEN2HEIGHT-1-Screen2->TopEdge);
   }

   else if (!(status=((++status)%4)))
      MoveScreen(Screen2,0,dheight-SCREEN2HEIGHT-1-Screen2->TopEdge);
   else if (status&1) MoveScreen(Screen2,0,dheight-1-Screen2->TopEdge);

   Forbid();
   if (svp2) svp2->DxOffset=(DESIRED_DX-curr_View->DxOffset)<<1;
   Permit();
   if (status==2) ScreenToBack(Screen2);
   else ScreenToFront(Screen2);

   return 0;
}

DrawConnection(vertex_pt v0,short dir) /* Connect the vertices */
{
   if (dir<0 || dir>3 || (!v0) || (!(v0->next[dir]))) return 0;

   SetDrMd(rastport, COMPLEMENT);
   SetAPen(rastport, OUTLINECOLOR);

   if (!(v0->code[dir]&TUNNEL)) {
      Move(rastport, v0->x,v0->y);
      Draw(rastport,v0->next[dir]->x,v0->next[dir]->y);
   }
   else
   {
      /* We always want to draw from left to right or top to bottom */
      /* Next two statements make sure that dir=={1,2} */
      if(v0->x < v0->next[dir]->x) v0=v0->next[dir],dir=(dir+2)&3;
      if(v0->y < v0->next[dir]->y) v0=v0->next[dir],dir=(dir+2)&3;

      Move(rastport, v0->x,v0->y);

      if(v0->x != v0->next[dir]->x)
      {
         Draw(rastport,maxx+border,v0->y);
         Move(rastport,minx-border,v0->y);
         Draw(rastport,v0->next[dir]->x,v0->y);
      }
      else /* if vertical tunnel */
      {
         Draw(rastport,v0->next[dir]->x,maxy+border);
         Move(rastport,v0->x,miny-border);
         Draw(rastport,v0->x,v0->next[dir]->y);
      }
   }
   linedrawn=!linedrawn;

   return 0;
}

NextStrip(short initialize)
{
   static curr_strip=0;

   if (Window2->FirstGadget) 
      RemoveGList(Window2,Window2->FirstGadget,1000);

   curr_strip = (++curr_strip)%NUM_STRIPS;
   if(initialize==STRIP_INIT) curr_strip=0;

   AddGList(Window2,&gadget[first_gad[curr_strip]],0,1000,NULL);
   SetAPen(statport,0); SetOPen(statport,0);
   SetRast(statport,0);
   RefreshGList(Window2->FirstGadget, Window2, NULL, 1000);

   return 0;
}

UpdateFlags(ULONG flags)
{
   for(i=firstflaggad;i<CREATEGAD;i++)
   {
      if((gadget[i].GadgetID&(~FLAGGADG))&flags)
         gadget[i].Flags|=SELECTED;
      else gadget[i].Flags&=~SELECTED;
   }

   return 0;
}

MakeEdgeGadDown()
{
   if((gadget[CREATEGAD].Flags&SELECTED)!=SELECTED ^ linedrawn)
      DrawConnection(sv,olddir);

   if (!(gadget[CREATEGAD].Flags&SELECTED))
        OffGadget(&gadget[TUNNELGAD],Window2,NULL);
   else  OnGadget(&gadget[TUNNELGAD],Window2,NULL);
   gadget[DELETEGAD].Flags &= ~SELECTED;
   UpdateFlags((gadget[CREATEGAD].Flags&SELECTED)?createflags:modflags);
   SetAPen(statport,0); SetOPen(statport,0);
   SetRast(statport,0);
   RefreshGList(Window2->FirstGadget,Window2,NULL,NUMGADS);
   DisplaySprites(sprites_on=gadget[DSgad].Flags & SELECTED);
   
   return 0;
}

DeleteEdgeGadDown()
{
   short opposite;
   vertex_pt v0;

   if (!linedrawn || !sv || olddir==-1) return 0;

   v0=sv->next[olddir];
   opposite = (olddir+2)&3;

   if(v0)
   {
      v0->next[opposite]=0;
      v0->code[opposite]=NULL;
   }
   sv->next[olddir]=NULL;
   sv->code[olddir]=0;

   CheckRedundant(v0);
   CheckRedundant(sv);

   DrawConnection(sv,olddir);
   sv=NULL; olddir=-1;

   /* Delete the edge, and if no edges remain on either vertex, or 
      neighboring vertex has two opposite edges, then delete those
      vertices */

   if (autodraw)
   {
      ResizeMaze(v_head);
      DrawMaze(DOTS_EXCLUDED);
   }

   return 0;
}

IsolateGhostBox()
{
   short cb_dir,i;
   vertex_pt v1,v2;

   if (top_box == NULL || !in_con) return 0;

   /* Check top_box for the dir of center_box */
   for (i=0;i<4;i++)
     if (top_box->next[i] == center_box) cb_dir = i;

   v1 = top_box->next[(cb_dir+1)&3];
   v2 = top_box->next[(cb_dir+3)&3];
   ecode = NO_MOD;
   connectum(v1,v2);
   connectum(v2,v1);

   return 0;
}

CheckRedundant(vertex_pt v0)  /* Deletes non-corner, non-junction verts */
{
   short count=0,i,j=0,k=0;

   if (!v0) return 0;

   for(i=0;i<4;i++)
   {
      if (v0->next[i])
        count++, j=i;
   }
   k=(j+2)&3;

   if (!count)
      DeleteVertex(v0);
   else if ((count==2) && v0->next[k] 
        && (v0->code[j]&DOTS)==(v0->code[k]&DOTS)
        && (v0 != pac_start1) && (v0 != pac_start2)
        && (v0 != gbv1) && (v0 != gbv2)
        && !(v0->vcode&ENERGIZER) 
        && !((v0->code[j]|v0->code[k])&(TUNNEL|FORBID|GHOST_1WAY))
        && !((v0->next[k]->code[j]|v0->next[j]->code[k])
                                  &(TUNNEL|FORBID|GHOST_1WAY)))
   /* If straight and dots the same and no energizer and no tunnel or 1way
      and not pac or ghost start */
   {
      ecode=v0->code[j];
      connectum(v0->next[k],v0->next[j]);
      connectum(v0->next[j],v0->next[k]);
      v0->next[k] = v0->next[j] = NULL;
      DeleteVertex(v0);
   }
   
   return 0;
}

HandleFlags(USHORT gadgetID)
{
   short i;

   if (linedrawn) DrawConnection(sv,olddir);

   if((gadget[CREATEGAD].Flags&SELECTED) && (gadgetID & FLAGGADG))
   {
      createflags ^= (gadgetID & ~FLAGGADG);
      return 1;
   }
   
   else if (gadgetID & FLAGGADG && !(gadgetID & TUNNEL)) /* Modify Edges */
   {
      modflags ^= (gadgetID & ~FLAGGADG);
      if (sv && olddir != -1)
      {
         sv->code[olddir]
            =modflags&(DOTS|TUNNEL|GHOST_1WAY|PACSTART|GHOSTSTART|FORBID);
         sv->next[olddir]->code[(olddir+2) & 3]
            = modflags & (DOTS | TUNNEL)
              | (sv->next[olddir]->code[(olddir+2) & 3] /* Keep codes */
                 & (GHOST_1WAY | FORBID));
         sv->vcode = modflags & ENERGIZER;
         /* Put in ghostbox shit. & pacstart */
         if (modflags & PACSTART)
         {
            for(i=0;i<4;i++) pac_start1->code[i]&=~PACSTART;

            pac_start1 = sv;
            pac_start2 = sv->next[olddir];
         }
         else if (modflags & GHOSTSTART) MakeGhostBox(gbv1=sv,gbv2=sv->next[olddir]);
         else if (gadgetID & GHOSTSTART)
         {
            DelGhostBox();
            gbv1=gbv2=NULL;
         }

         if(CheckRedundant(sv->next[olddir])) olddir=-1;
         if(CheckRedundant(sv))
         {
            sv=NULL;
            olddir=-1;
         }
         else if (autodraw)
         {
            ResizeMaze(v_head);
            DrawMaze(DOTS_EXCLUDED);
         }
         DisplaySprites(sprites_on);
      }

      return 1;
   }
   if (!linedrawn) DrawConnection(sv,olddir);

   return 0;
}

DisplaySprites(short doit)
{
   short i;

   if (!((mazestatus&GRAPH_LOADED)&&(mazestatus&VARS_LOADED)&&doit))
   {
      for(i=1;i<8;i++) FreeSprite(i);

      return 0;
   }
   else
   {
      SetupSprites();
      InitializeStarting();
      PosAllSprite();
      SetMazeColors();
   }

   return 0;
}

DrawRubBand(short x1, short y1, short x2, short y2, SHORT tuncode)
{
   SetDrMd(rastport, COMPLEMENT);
   Move(rastport, x1, y1);
   if (!((x1 != x2)? tuncode & (TUN_LEFT | TUN_RIGHT)
              : tuncode & (TUN_UP | TUN_DOWN)))
      Draw(rastport, x2, y2);
   else /* is tunnel */
   {
      if ((x1 < x2) || (y1 < y2))
      {
         XORSWAP(x1,x2);
         XORSWAP(y1,y2);
         Move(rastport, x1, y1);  
      }

      if (x1 != x2)
      {
         Draw(rastport,maxx+border,y1);
         Move(rastport,minx-border,y1);
         Draw(rastport,x2,y1);
      }
      else
      {
         Draw(rastport,x1,maxy+border);
         Move(rastport,x1,miny-border);
         Draw(rastport,x1,y2);
      }
   }
   return 0;
}

FindTunDirs(short x1, short y1)
{
   short tuncode=TUN_UP|TUN_DOWN|TUN_LEFT|TUN_RIGHT;
   short a, tempxy, step;
   ULONG color;

   for (a=0;a<2;a++)
   {
      step = a? -xfactor : xfactor;
      for (tempxy = x1 + step; 
           a? (tempxy >= minx) : (tempxy <= maxx);
           tempxy += step)
      {
         color = ReadPixel(rastport, tempxy,y1);
         if ((color == DOTCOLOR) || (color == ENERGCOLOR))
         {
            tuncode &= (a? ~TUN_LEFT : ~TUN_RIGHT);
            break;
         }
      }
   }

   for (a=0;a<2;a++)
   {
      step = a? -yfactor : yfactor;
      for (tempxy = y1 + step; 
           a? (tempxy >= miny) : (tempxy <= maxy);
           tempxy += step)
      {
         color = ReadPixel(rastport, x1, tempxy);
         if ((color == DOTCOLOR) || (color == ENERGCOLOR))
         {
            tuncode &= (a? ~TUN_UP : ~TUN_DOWN);
            break;
         }
      }
   }

   if ((tuncode & TUN_UP) && (tuncode & TUN_DOWN))
     tuncode |= TUN_UPDOWN;
   if ((tuncode & TUN_LEFT) && (tuncode & TUN_RIGHT))
     tuncode |= TUN_RIGHTLEFT;

     #if MYDEBUG
     printf("tuncode: %x\n",tuncode);
     #endif
   
   return (int) tuncode;
}      

DrawToVert(short x1,short y1,short *x2,short *y2, short tuncode)
{
   short step, tempxy;
   ULONG color;
   
   #if MYDEBUG
   printf("tuncode in DrawToVert: %x\n",tuncode);
   #endif


   if (ABS(x1 - *x2) > ABS(y1 - *y2))      /* If dx greater */
   {
      *y2 = y1;
      SetDrMd(rastport, COMPLEMENT);
      Move(rastport, x1, y1);
      step = (x1 > *x2) ? -xfactor : xfactor;
      if (tuncode & (TUN_LEFT | TUN_RIGHT))
      {
         Draw(rastport,(tuncode &TUN_RIGHT)? maxx+border : minx-border,y1);
         Move(rastport,(tuncode &TUN_RIGHT)? minx-border : maxx+border,y1);
         x1=(tuncode & TUN_RIGHT)? minx-xfactor : maxx+xfactor;
         step = (tuncode&TUN_RIGHT)? xfactor: -xfactor;
      }

      color = ReadPixel(rastport,x1+step,*y2);
      if (((color == DOTCOLOR) || (color == ENERGCOLOR)) 
          && !(tuncode & (TUN_LEFT | TUN_RIGHT)))
         *x2 = x1;
      else for (tempxy = x1 + step;
                tempxy != *x2; 
                tempxy += step)
      {
         color = ReadPixel(rastport, tempxy,*y2);
         if ((color == DOTCOLOR) || (color == ENERGCOLOR))
         {
            *x2 = tempxy;
            break;
         }
      }
   }
   else  /* dy greater */
   {
      *x2 = x1;
      SetDrMd(rastport, COMPLEMENT);
      Move(rastport, x1, y1);
      step = (y1 > *y2) ? -yfactor : yfactor;
      if (tuncode & (TUN_UP | TUN_DOWN))
      {
         #if MYDEBUG
         printf("dy > dx: tuncode & UP/DOWN: %x"
                ,(tuncode & TUN_UP | TUN_DOWN));
         #endif
         Draw(rastport,*x2,(tuncode&TUN_DOWN)? maxy+border : miny-border);
         Move (rastport,x1,(tuncode&TUN_DOWN)? miny-border : maxy+border);
         y1=(tuncode&TUN_DOWN)? miny-yfactor : maxy+yfactor;
         step = (tuncode&TUN_DOWN)? yfactor: -yfactor;
      }

      color = ReadPixel(rastport,x1,y1+step);
      if (((color == DOTCOLOR) || (color == ENERGCOLOR))
          && !(tuncode & (TUN_UP | TUN_DOWN)))
         *y2 = y1;
      else for (tempxy = y1 + step;
                tempxy != *y2; 
                tempxy += step)
      {
         color = ReadPixel(rastport, *x2,tempxy);
         if ((color == DOTCOLOR) || (color == ENERGCOLOR))
         {
            *y2 = tempxy;
            break;
         }
      }
   }
   if ((x1 != *x2) || (y1 != *y2)) /* If not in start square */
      Draw(rastport, *x2, *y2);  
      
   return 0;
}
