#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <libraries/arpbase.h>
#include <graphics/gfxmacros.h> /* SetOPen Macro */
#include <graphics/sprite.h>             /* needed for sprite functions */

#include "pacdef.h"

#define global extern
#include "pcon.h"
#include "pacvar.h"
#include "keydef.h"

#define FLAGGADG 0x8000
#define MAXSTRLEN       16
#define NUMGADS         23
#define NUM_STRIPS      3
#define CREATEGAD       8
#define DELETEGAD       9
#define DOTGAD          1
#define TUNNELGAD       2

extern struct IntuiText gadgetintui[NUMGADS];
extern struct Border gadgetborder[2][MAXSTRLEN*2];
extern struct Gadget gadget[NUMGADS];
extern SHORT gadgetxy[MAXSTRLEN*2][10];
extern first_gad[NUM_STRIPS];
extern char *gadgettxt[NUMGADS];
extern USHORT gadID[NUMGADS];

handleIDCMP2( struct Window *win )  /* Lower Screen */
/* Handle the IDCMP messages */
{
   USHORT code;
   struct IntuiMessage *message = NULL;
   ULONG  class;
   APTR iaddress;
   USHORT gadgetID;

   /* Examine pending messages */
   while(message = (struct IntuiMessage *)GetMsg(win->UserPort))
   {
      class = message->Class;
      code = message->Code;
      iaddress = message->IAddress;

      /* When we're through with a message, reply */
      ReplyMsg((struct Message *)message);

      /* See what events occurred */
      switch( class )
      {
         case MOUSEBUTTONS:
            switch (code)
            {
               case MENUUP:
                  ToggleScreenToFront(0);
                  break;
               default: break;
            }
         case RAWKEY:
            if (HandleKeys(code)) return 1;
            break;

         case RELVERIFY:
            break;

         case GADGETDOWN:
            gadgetID = ((struct Gadget *)iaddress)->GadgetID;

            if(!HandleFlags(gadgetID)) switch (gadgetID)
            {
               case NEXTSTRIP:
                  NextStrip(STRIP_NEXT);
                  break;
               case MAKEEDGE:
                  MakeEdgeGadDown();
                  break;
               case DELEDGE:
                  DeleteEdgeGadDown();
                  break;
               case QUITMAZED:
                  return 1;
               case LOADMAZE:
                  LoadMaze();
                  win=PlayWindow;
                  break;  /* Our window may not be good anymore! */
               case SAVEMAZE:
                  SaveGraph();
                  break;
               case LOADVARS:
                  LoadVars();
                  win=PlayWindow;
                  break; /* Our window may not be good anymore! */
               case DRAWMAZE:
                  ResizeMaze(v_head);
                  DrawMaze(DOTS_EXCLUDED);
                  break;
               case AUTODRAW:
                  autodraw = gadget[ADgad].Flags & SELECTED;
                  break;
               case DISPLAY_SPRITES:
                  DisplaySprites(sprites_on=gadget[DSgad].Flags & SELECTED);
                  break;
               default:
                  break;
            }
            break;

         case INTUITICKS:
            /* Do drawing routines or whatever... */
            break;

         default:
            break;
      } /* end switch */
   } /* end while */
   return 0;
}

handleIDCMP1b( struct Window *win )    /* Construction Screen */
/* Handle the IDCMP messages */
{
   USHORT code;
   struct IntuiMessage *message = NULL;
   ULONG  class;
   APTR iaddress;
   static SHORT oldx, oldy,x1,x2,y1,y2,mousex,mousey, tuncode;
   static vertex_pt v0,v1;
   char dir;

   /* Examine pending messages */
   while(message = (struct IntuiMessage *)GetMsg(win->UserPort))
   {
      class = message->Class;
      code = message->Code;
      iaddress = message->IAddress;
      mousex = message->MouseX + rasInfo->RxOffset;
      mousey = message->MouseY + rasInfo->RyOffset;

      /* When we're through with a message, reply */
      ReplyMsg((struct Message *)message);

      /* See what events occurred */
      if (gadget[CREATEGAD].Flags & SELECTED) switch( class )
      {
         case MOUSEBUTTONS:      /* Create */
            switch (code)
            {
               case SELECTDOWN:
                  RoundXY(mousex,mousey);
                  x1 = x2 = BOUND(mousex,minx,maxx);
                  y1 = y2 = BOUND(mousey,miny,maxy);
                  ModifyIDCMP(win, MOUSEMOVE | win->IDCMPFlags);
                  followmouse = 1;
                  if (gadget[TUNNELGAD].Flags&SELECTED)
                  {
                     if (!(tuncode = FindTunDirs(x1,y1)))
                     {
                        createflags &= ~TUNNEL;
                        UpdateFlags(createflags);
                        SetAPen(statport,0); SetOPen(statport,0);
                        SetRast(statport,0);
                        RefreshGList(Window2->FirstGadget,Window2
                                    ,NULL,NUMGADS);
                     }
                  }
                  else tuncode=0;
                  break;
               case SELECTUP:
                  followmouse = 0;
                  ModifyIDCMP(win, (~MOUSEMOVE) & win->IDCMPFlags);
                  if ((x1 != x2) || (y1 != y2))   /* If not in start sqr */
                  {
                     SetDrMd(rastport, COMPLEMENT);   /* Draw Path */
                     Move(rastport, x1, y1);
                     Draw(rastport, x2, y2);

                     v0 = AddConnectVert(x1,y1, (x1==x2));
                     v1 = AddConnectVert(x2,y2, (x1==x2));

                     if(createflags&ENERGIZER) v0->vcode|=ENERGIZER;

                     ecode = createflags;
                     connectum(v0,v1);
                     connectum(v1,v0);

                     if (x1 > maxx) maxx = x1;
                     if (x1 < minx) minx = x1;
                     if (y1 > maxy) maxy = y1;
                     if (y1 < miny) miny = y1;

                     if (x2 > maxx) maxx = x2;
                     if (x2 < minx) minx = x2;
                     if (y2 > maxy) maxy = y2;
                     if (y2 < miny) miny = y2;

                     #if MYDEBUG
                     printf("min/max x/y: %d %d %d %d\n",minx,maxx,miny,maxy);
                     #endif

                     dir = (x2>x1)? 1: (x2<x1)? 3: (y2>y1)? 2: 0;
                     if (createflags & TUNNEL)
                        dir = (dir +2) & 3;
                     if (createflags & PACSTART)
                     {
                        for(i=0;i<4;i++) pac_start1->code[i]&=~PACSTART;
                        pac_start1=v0,pac_start2=v1;
                     }
                     v0->code[dir] = createflags
                        & (DOTS|TUNNEL|GHOST_1WAY|FORBID|PACSTART);
                     v1->code[(dir+2)&3] = createflags
                        & (DOTS|TUNNEL);
                     CheckRedundant(v0);
                     CheckRedundant(v1);

                     SetDrMd(rastport, JAM1);   /* Draw Path */
                     Move(rastport, x1, y1);
                     SetAPen(rastport, 
                     (createflags & DOTS) ? DOTCOLOR : ENERGCOLOR);
                     Draw(rastport, x2, y2);

                     if (createflags & GHOSTSTART)
                        MakeGhostBox(gbv1=v0,gbv2=v1);
                     if (autodraw)
                     {
                        ResizeMaze(v_head);
                        DrawMaze(DOTS_EXCLUDED);
                     }
                  }
                  break;
               case MENUUP:
                  ToggleScreenToFront(0);
                  break;
               default: break;
            }
            break;
         case MOUSEMOVE:
            if (!followmouse) break;
            RoundXY(mousex, mousey);
            if ((oldx != mousex) || (oldy != mousey))     /* If mouse moved */
            {
               if ((x1 != x2) || (y1 != y2))  /* If not in start square */
                   DrawRubBand(x1,y1,x2,y2,tuncode);
                                                   /* Erase rubber Band */

               oldx = x2 = mousex;
               oldy = y2 = mousey;
               if ((x1 != x2) || (y1 != y2))          /* Note: New x2,y2 */
                                               /* If not in start square */
                     DrawToVert(x1,y1,&x2,&y2,tuncode);
            }
         break;
         case RAWKEY:
            if (HandleKeys(code)) return 1;
            win=PlayWindow;
            break;

         case RELVERIFY:
            break;

         case GADGETDOWN:
            break;

         case INTUITICKS:
            /* Do drawing routines or whatever... */
            break;

         default:
            break;
      } /* end switch */
      else switch( class ) /* Modifying or deleting... */
      {
         case MOUSEBUTTONS:
            switch (code)
            {
               case SELECTDOWN:
                  if (linedrawn) DrawConnection(sv,olddir);
                  RoundXY(mousex,mousey);
                  if (sv=get_vertex(mousex,mousey))
                     ModifyIDCMP(win,
                        SELECTUP|MOUSEMOVE|INTUITICKS|win->IDCMPFlags);
                  else break;
                  x1 = x2 = mousex;
                  y1 = y2 = mousey;
                  olddir = -1;
                  doflashing = 1;
                  linedrawn = 0;
                  break;
               case SELECTUP:
                  ModifyIDCMP(win,~(MOUSEMOVE|INTUITICKS)&win->IDCMPFlags);

                  if (!sv)
                     break;
                  if (olddir != -1)
                  {
                     modflags = sv->code[olddir]&(~(ENERGIZER));
                     modflags |= sv->vcode;
                     UpdateFlags(modflags);
                     RefreshGList(Window2->FirstGadget,Window2,NULL,1000);
                           
                     /* Draw the line! (Rubber Band) */
                     if (!linedrawn) DrawConnection(sv,olddir);

                     doflashing=0;
                  }
                  break;
               case MENUUP:
                  ToggleScreenToFront(0);
                  break;
               default: break;
            }
            break;
         case MOUSEMOVE:
            if (!sv)
               break;
            RoundXY(mousex, mousey);
            if ((x2 != mousex) || (y2 != mousey))     /* If mouse moved */
            {
               if ((x1 != mousex) || (y1 != mousey))  /* ! in start sqr */
               {
                  dir = ( (ABS(x1 - mousex) > ABS(y1 - mousey)) ?
                          ((mousex > x1) ? 1 : 3) :
                          ((mousey > y1) ? 2 : 0) );
                  if (dir != olddir)                 /* If moved */
                  {
                     if (linedrawn)  /* Erase old Rubber Band */
                        DrawConnection(sv,olddir);
                     
                     olddir = dir;

                     if (sv->next[dir]) /* Draw the new one */
                     {
                        DrawConnection(sv,dir);

                        x2 = sv->next[dir]->x;
                        y2 = sv->next[dir]->y;
                        
                     }
                     else olddir = -1;
                  }
               }
               else /* In start Square */
               {
                  if (linedrawn)
                     DrawConnection(sv, olddir);

                  olddir = -1;
               }
            }
         break;
         case RAWKEY:
            if (HandleKeys(code)) return 1;
            win=PlayWindow;
            break;

         case RELVERIFY:
            break;

         case GADGETDOWN:
            break;

         case INTUITICKS:
            /* Do drawing routines or whatever... */
            if (!sv)
               break;
            if (olddir != -1 && doflashing)
               DrawConnection(sv,olddir);
                     
            break;

         default:
            break;
      } /* end switch */
   } /* end while */
   return 0;
}

HandleKeys(USHORT code)
{
   if(code>0 && code < (CREATEGAD-firstflaggad+1) &&
    !(gadget[code+firstflaggad-1].Flags & GADGDISABLED))
   {
      /* This will let you Select flags w/nums */
      gadget[code+firstflaggad-1].Flags^=SELECTED;
      RefreshGList(Window2->FirstGadget,Window2,NULL,NUMGADS);
      HandleFlags(gadget[code+firstflaggad-1].GadgetID);
   }
   else switch(code)
   {
      case LEFT_KEY:
         Scrollit(-xfactor,0);
         break;
      case RIGHT_KEY:
         Scrollit(xfactor,0);
         break;
      case UP_KEY:
         Scrollit(0,-yfactor);
         break;
      case DOWN_KEY:
         Scrollit(0,yfactor);
         break;

      case Q_KEY: /* Q - Quit */
         return 2;
         break;
      case M_KEY:
         LoadMaze();
         break;
      case S_KEY:
         SaveGraph();
         break;
      case V_KEY:
         LoadVars();
         break;
      case D_KEY:
         ResizeMaze(v_head);
         DrawMaze(DOTS_EXCLUDED);
         break;
      case C_KEY:
         gadget[CREATEGAD].Flags ^=  SELECTED;
         MakeEdgeGadDown();
         break;
      case DELETE_KEY:
      case BACKSP_KEY:
      case E_KEY:
         DeleteEdgeGadDown();
         break;
      case SPACEBAR:
         ToggleScreenToFront(0);
         break;
      case TAB_KEY:
         NextStrip(STRIP_NEXT);
         break;
      default:
         break;
   } /* End switch key rawcode */

   return 0;
}

LoadMaze()
{
   short tx=rasInfo->RxOffset,ty=rasInfo->RyOffset;
   
   Scrollit(-tx,-ty);
   DisplaySprites(FALSE);

   fileRequester->fr_Hail="Choose a graph file";
   strcpy(fileRequester->fr_Dir,"pw:graph");
   strcpy(fileRequester->fr_File,"");
   LoadColors(&(PlayScreen->ViewPort),FR_COLORS);
   ScreenToBack(Screen2);

   if(FileRequest(fileRequester))
   {
      strcpy(fr_path,fileRequester->fr_Dir);
      strcpy(fr_name,fileRequester->fr_File);
      strcpy(filename,fr_path);

      if ( !strchr("/:\0",fr_path[strlen(fr_path)-1]) && strlen(fr_path))
         strcat(filename,"/");

      if (strlen(filename) > 1 && strlen(fr_name))
      {
         strcat(filename,fr_name);
         if(!ReadGraph(filename))
         {
            width=height=0;
            ResizeMaze(v_head);
            linedrawn=0; olddir=-1;sv=NULL;
            DrawMaze(DOTS_EXCLUDED);
         }
      }
   }

/*   ScreenToFront(PlayScreen); */
   DisplaySprites(sprites_on);
/*   ToggleScreenToFront(1); */

   return 0;
}

LoadVars()
{
   short tx=rasInfo->RxOffset,ty=rasInfo->RyOffset;
   
   Scrollit(-tx,-ty);
   DisplaySprites(FALSE);

   fileRequester->fr_Hail="Choose a variable file";
   strcpy(fileRequester->fr_Dir,"pw:vars");
   strcpy(fileRequester->fr_File,"");
   LoadColors(&(PlayScreen->ViewPort),FR_COLORS);
   ScreenToBack(Screen2);
                     
   if(FileRequest(fileRequester))
   {
      strcpy(fr_path,fileRequester->fr_Dir);
      strcpy(fr_name,fileRequester->fr_File);

      strcpy(filename,fr_path);

      if ( !strchr("/:\0",fr_path[strlen(fr_path)-1]) && strlen(fr_path))
         strcat(filename,"/");

      if (strlen(filename) > 1 && strlen(fr_name))
      {
         strcat(filename,fr_name);

         if (!ReadHeader(filename))
         {
            width=height=0;
            ResizeMaze(v_head);
            linedrawn=0; olddir=-1;sv=NULL;
            DrawMaze(DOTS_EXCLUDED);
         }
      }
   }

/*   ScreenToFront(PlayScreen); */
   DisplaySprites(sprites_on);
/*   ToggleScreenToFront(1); */


   return 0;
}

Scrollit(short x,short y)
{
   static short px=0,py=0;

   px+=x;
   py+=y;

   if(px+WIDTH > width) rasInfo->RxOffset=(px=width-WIDTH)/*&~15*/;
   else if(px < 0)   px=rasInfo->RxOffset=0;
   else                 rasInfo->RxOffset=(px/*&~15*/);

   if(py+dheight>height+1) rasInfo->RyOffset=py=height+1-dheight;
   else if(py < 0)     py=rasInfo->RyOffset=0;
   else                   rasInfo->RyOffset=py;

   MakeVPort(curr_View,viewPort);
   MrgCop(curr_View);
   LoadView(curr_View);
   PosAllSprite();

#if MYDEBUG
   PrintColorMapInfo(PlayScreen);
#endif

   return 0;
}
