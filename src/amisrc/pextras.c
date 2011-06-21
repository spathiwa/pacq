/*** PEXTRAS.C
   Try to keep routines that are expected to work, and should not be
   modified in this source file. */

#define global extern

#include <exec/types.h>            /* Amiga specific variable type declarations */
#include <graphics/sprite.h>             /* needed for sprite functions */

#include "pacdef.h"
#include "pacvar.h"

extern struct SimpleSprite sprite[MAXGHOSTS+1];
extern struct ViewPort *statViewPort;
extern USHORT scols2[2];
extern short gint[MAXGHOSTS];

WrapSprite(short sprnum,short mx,short my)
{
   if (superx && mx && !(xposn[sprnum]+XOFF<width &&
        xposn[sprnum]+XOFF>=0))
   {
       xposn[sprnum]+=(mx>0)? -(width):(width);
       if(!sprnum) xras_dest=(mx>0)? width : 0;
       PosSprite(sprnum);
   }
   else if (mx && !(xposn[sprnum]+XOFF<=maxx+(xfactor<<1) &&
        xposn[sprnum]+XOFF>=minx-(xfactor<<1)))
   {
       xposn[sprnum]+=(mx>0)? -(maxx-minx+(xfactor<<2))
                               :(maxx-minx+(xfactor<<2));
       PosSprite(sprnum);
       if (superx && !sprnum) ScrollDisplay(SCROLLTUNNEL);
       /* If not ghost, then definitely scroll. */
   }
   else if (my && !(yposn[sprnum]+YOFF<=maxy+(yfactor<<1) &&
        yposn[sprnum]+YOFF>=miny-(yfactor<<1)))
   {
       yposn[sprnum]+=(my>0)? -(maxy-miny+(yfactor<<2))
                               :(maxy-miny+(yfactor<<2));
       PosSprite(sprnum);
       if (supery && !sprnum) ScrollDisplay(SCROLLTUNNEL);
       /* If not ghost, then definitely scroll. */
   }

   return 0;
}

BlinkMonster(short monstnum,BOOLEAN blink)
{
 SetRGB4(svp,17+(spgot[monstnum+1]<<1),(scols2[blink] & 0xF00)>>8,
                       (scols2[blink] & 0x0F0)>>4,scols2[blink] & 0x00F);
 if(!wb2_0)
    SetRGB4(statViewPort,17+(spgot[monstnum+1]<<1),(scols2[blink] & 0xF00)>>8,
                       (scols2[blink] & 0x0F0)>>4,scols2[blink] & 0x00F);
                       
 return 0;
}

Energize()                   /* Call this when PACMAN eats an energizer */
{
 register int e;

 half=15;
 bonus=200;          /* Score currently will receive for eating a ghost */
 scoff=0;

 num_energized = 0;

 for(e=0;e<numghosts;e++)      /* energized[x] decremented in moveghost */
 {
   if(energized[e]>=0) 
   {
     energized[e]=energtime;
     if (!inbox[e]) changespeed(e,bluespeed);
   }
   num_energized++;
   /* >=0 because we don't want to energize the ghosts returning home! */
 }

 for(e=0;e<numghosts;e++) BlinkMonster(e,0);

 ReverseGhosts(REV_ENERG);
 killallsound();
 playsound(SND_ENERGIZE,0);
 playsound(SND_ENERGIZE,0);

 return 0;
}

DeEnergize(int gstnum,BOOLEAN norm)    /* Call when energized[gstnum]<1 */
{
  /* norm == 1 : time ran out ; norm == 2 : ghost returns to ghostbox */

 dead=0;
 if (norm)                     /* Out of time, or returned to ghost box */
 {
  inbox[gstnum] = (energized[gstnum]<0);
  if (!inbox[gstnum] && !stay[i]) changespeed(gstnum,basespeed[gstnum]);
  else changespeed(gstnum,boxspeed);

  energized[gstnum]=0;
  NormGhostColor(gstnum);
  num_energized--;      /* Tell that there is one less energized ghost. */
  if(norm>1) numeyes--;
  if (numeyes<1)
  {
    killallsound();
    if (num_energized)
    {
      playsound(SND_ENERGIZE,0);
      playsound(SND_ENERGIZE,0);
    }
    else playsound(SND_BACKGRND,0),playsound(SND_BACKGRND,0);
  }
 }
 else            /* This routine is for when a ghost is eaten by PACMAN */
 {
   DisplayBonusScore(gstnum);
   energized[gstnum]=-1;                  /* This says "GO HOME GHOST!" */
   changespeed(gstnum,eyespeed);         
   numeyes++;                 /* How many eyes must return to ghost box */
 }

 return 0;
}

changespeed(int gstnum, int spd)
{
  speed[gstnum]=spd&(NUMSPEEDS-1);
  movefast[gstnum]=(short) spd / NUMSPEEDS; 

  return 0;
}

ReverseGhosts(int rule)    /* Called from energize and at regular time intervals */
{
 register short i;/* Auto var takes prec over global - wanted to use [i] */

 for(i=0;i<numghosts;i++)
   revtoggle[i]=(rule==REV_TIME) ? !revtoggle[i] : (!revtoggle[i])<<1;
                                  /* TOGGLE_TIME : TOGGLE_ENERG */
 revover = 1;

 return 0;
}

NormGhostColor(short gnumber)
{
 int cn = 17 + ((spgot[gnumber+1])<<1);
   SetRGB4(svp,cn,
         colorvalue[cn][R_RGB],colorvalue[cn][G_RGB],colorvalue[cn][B_RGB]);

 if(!wb2_0)
   SetRGB4(statViewPort,cn,
       colorvalue[cn][R_RGB],colorvalue[cn][G_RGB],colorvalue[cn][B_RGB]);

 return 0;
}

DisplayBonusScore(short monstnum)
{
 short spnum;

 spnum=monstnum+1;
 INCSCORE(bonus);
 bonus<<=1;
 ChangeSprite(svp,&sprite[0],(void *)blank);
 ChangeSprite(svp,&sprite[spnum],(void *)(scores+scoff));
 scoff+=1;
 
 Delay(25); /* This waits for MAX(2 seconds,time to play die sound) */
 while(soundplaying(SND_MONSTER))
  ;
 
 
 ChangeSprite(svp,&sprite[0],(void *)(pacmen+spoff));

 return 0;
}
