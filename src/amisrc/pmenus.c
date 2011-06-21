/*** PMENUS.C - This is the module that does menu selection/construction */
/*
/*************************************************************************/

#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <libraries/arpbase.h>

extern struct FileRequester *fileRequester;
extern struct TextFont *OpenFont(struct TextAttr *);
extern struct TextFont *LoadThisFont(struct TextAttr *);
extern struct ArpBase *ArpBase;
extern struct Window *Window;
extern char def_title[],titlebuff[];

#define global  /* Nullify the global definition to allow menu defintion */

#include "pacdef.h"
#include "pmenus.h"

struct IntuiText bodytext[]=
{
  { 1,0,JAM2,26, 3,0," WARNING: PACQ Construction Set ",&bodytext[1]},
  { 1,0,JAM2,26,11,0," is currently in the Beta stage ",&bodytext[2]},
  { 1,0,JAM2,26,19,0," of development..  ..Proceed?   ",0}
},
negtext=
{
   1,0,JAM2,6,3,0,"Forget you!",0
},
postext=
{
   1,0,JAM2,6,3,0,"Awwww, well, Okay...",0
};

unsigned char curr_filter;

handleIDCMP( struct Window *win )
/* Handle the IDCMP messages */
{
   static char warned=0;
   int  flag = 0;
   USHORT code, selection, flags;
   struct IntuiMessage *message = NULL;
   ULONG  class, menuNum, itemNum, subNum;

   if(strcmp(titlebuff,def_title))
   {
      strcpy(titlebuff,def_title);
      SetWindowTitles(Window,titlebuff,titlebuff);
   }

   /* Examine pending messages */
   while(message = (struct IntuiMessage *)GetMsg(win->UserPort))
   {
      class = message->Class;
      code = message->Code;

      /* When we're through with a message, reply */
      ReplyMsg((struct Message *)message);

      /* See what events occurred */
      switch( class )
      {
         case MENUPICK:
            selection = code;
            while(selection != MENUNULL)
            {
               menuNum = MENUNUM(selection);
               itemNum = ITEMNUM(selection);
               subNum  = SUBNUM(selection);
               flags = (int) ((struct MenuItem *)
                       ItemAddress(FirstMenu,(LONG)selection))->Flags;
               if(flags&CHECKED) ;
               switch( menuNum )
               {
                  case PROJ:    /* Project Menu */
                     switch(itemNum)
                     {
                        case PROJ_ABOUT:
                           break;
                        case PROJ_PLAY:
                           strcpy(fileRequester->fr_Hail,"Choose a game");
                           strcpy(fileRequester->fr_Dir,"pw:game");
                           strcpy(fileRequester->fr_File,"");
                           flag = (int) FileRequest(fileRequester);
                           break;
                        case PROJ_CONST:
                           if(!warned)
                           {
                              warned=AutoRequest(Window,&bodytext[0],
                                       &postext,&negtext,0L,0L,320,61);
                           }
                           if(warned)
                           {
                              Construction();
                              win = Window;
                           }
                           break;
                        case PROJ_QUIT:
                           FreeMemory();
                           break;
                        default:
                           break;
                     } /* end switch */
                     break;
                  case 1:    /* Screen Menu */
                     switch(itemNum)
                     {
                        case 0:    /* Filter */
                           curr_filter = 
                               (unsigned char) FilterOff(curr_filter);
                           playsound(SND_CREDIT,1);
                           break;
                        default:
                           break;
                     } /* End switch for which item in Screen Menu */
                     break;
                  default:
                     break;
               } /* end switch */
               selection = ((struct MenuItem *)
                 ItemAddress(FirstMenu,(LONG)selection))->NextSelect;
            } /* end while */
            break; /* case of MENUPICK */
      
         case RAWKEY:
            switch(code)
            {
               case 0x10: /* Q - Quit */
               /* FreeMemory(); */
                  break; /* took this out to prevent unintended quit */
               default:
                  break;
            } /* End switch key rawcode */

         case INTUITICKS:
            /* Do drawing routines or whatever... */
            break;

         default:
            break;
      } /* end switch */
   } /* end while */
   return(flag);
}

AdjustMenus(struct Menu *firstmenu, struct TextAttr *attr)
{
   struct RastPort textrp= {0};
   struct Menu             *menu;
   struct TextFont         *font;
   USHORT                  start, width, height, space;
   int                     retval = 0;

   if((font = LoadThisFont(attr)))
   {
      SetFont(&textrp,font);
      width = font->tf_XSize;

      height=(font->tf_YSize < 8) ? 8:font->tf_YSize;
      height++;
      
      start = 2;
      
      menu = firstmenu;
      while(menu)
      {
         menu->LeftEdge = start;
         menu->Width = space = TextLength(&textrp,menu->MenuName,
                                          (LONG)strlen(menu->MenuName)) + width;
         AdjustItems(&textrp,menu->FirstItem,attr,width,height,0,0);
         menu = menu->NextMenu;
         start+= (space + (width*2));
      }
      CloseFont(font);
      
      retval = 1;
   }
   return(retval);
}

AdjustItems(struct RastPort *txtrp, struct MenuItem *fi,
            struct TextAttr *atr, USHORT wdth, USHORT hght,
            USHORT lvl, USHORT edge)
{
  struct   MenuItem *item = fi;
  register USHORT num;
  USHORT   strip_width, sub_edge;
  
  if(fi==NULL) return 0;
  strip_width = MaxLength(txtrp, item, wdth);
  num = 0;
  while(item)
  {
    item->TopEdge = (num * hght) - lvl;
    item->LeftEdge = edge;
    item->Width = strip_width;
    item->Height = hght;
    sub_edge = strip_width - wdth;
    AdjustText((struct IntuiText *)item->ItemFill, atr);
    AdjustItems(txtrp, item->SubItem, atr, wdth, hght, 1, sub_edge);
    item = item->NextItem;
    num++;
  }
      
  return 0;
}

MaxLength(struct RastPort *txtrp, struct MenuItem *fi, USHORT width)
/* Steps thru each item to determine the maximum width of the strip */
{
   int      maxval = 0;
   USHORT   textlen;
   struct   MenuItem  *item = fi;
   struct   IntuiText *itext;

   while(item)
   {
      if(item->Flags&COMMSEQ)
      {
        width += (width + COMMWIDTH);
        break;
      }
      item = item->NextItem;
   }
   item = fi;
   while(item)
   {
      itext = (struct IntuiText *)item->ItemFill;
      textlen = itext->LeftEdge +
         TextLength(txtrp,itext->IText,(LONG)strlen(itext->IText)) + width;
      /* returns the greater of the two */
      maxval = (textlen<maxval)?maxval:textlen;
      item = item->NextItem;
   }
   return(maxval);
}

AdjustText(struct  IntuiText *text, struct  TextAttr *attr)
/* Adjust the MenuItems font attribute */
{
   struct IntuiText *nt;
   nt = text;
   while(nt)
   {
      nt->ITextFont = attr;
      nt = nt->NextText;
   }

   return 0;
}

FilterOff(unsigned char off) 
/* Pass non-zero value to turn off the low-pass audio filter */
{
   unsigned short *filter = (unsigned short *)0xBFE000;

   if(off) *filter = 0x7002;
   else    *filter = 0x7000;

   return !off;
}
