/*** PGHOST.C
   All the ghost movement routines */

#include <stdio.h>            /* standard input/output library (printf) */
#include <exec/types.h>            /* Amiga specific variable type declarations */
#include <graphics/view.h>
#include <graphics/sprite.h>             /* needed for sprite functions */

#define global extern

#include "pacdef.h"
#include "pacvar.h"

global struct SimpleSprite sprite[MAXGHOSTS+1]; /* sprite cntl structure */
global struct ViewPort *viewPort;
global short gint[MAXGHOSTS];
global USHORT speedlist[NUMSPEEDS];

MoveGhosts()
{
 int pref,pref2,pref3,consider,xgty,choice,tempchoice,gdx,gdy,
       mdistx,mdisty,mdist;                            /* move distance */
 int look,numchoices;
 int gotem=0;

 half=(half+1)&16383; /* Flag for half speed ghosts and energizer blinks */
 if(!(half&2047) || (!revover && (half == firstrev)))
   ReverseGhosts(REV_TIME);  /* Reverse Ghost every once in while */

 if(!(half&15))
  BlinkEnergizer(!(half&31));                       /* Blink energizer. */

 for(i=0;i<numghosts;i++)
 {
  c=i+1;                                          /* Sprite Index value */
  gdiri=gdir_code[i]=((gy[i]>0)?2:0)+(gx[i]>0)+((gx[i]<0)?3:0);
  mdisty=mdistx=0;

  if(speedlist[speed[i]] & (1 << (half & (NUMSPEEDS-1) )))
  {
    mdistx=gx[i];
    mdisty=gy[i];
  }

  if (movefast[i])
  {
    mdistx+=(gx[i])?(movefast[i])*gx[i]:0;
    mdisty+=(gy[i])?(movefast[i])*gy[i]:0;
  }

                                      /* mdistx,mdisty = +/- 0, 1, 2... */

  while (ABS(gdest_v[i]->y-YOFF-yposn[c]) <= ABS(mdisty) &&
        ABS(gdest_v[i]->x-XOFF-xposn[c]) <= ABS(mdistx))
  {
   mdisty -= gdest_v[i]->y-YOFF-yposn[c];              /* subtract dist */
   mdistx -= gdest_v[i]->x-XOFF-xposn[c];                  /* to vertex */

   yposn[c]=gdest_v[i]->y-YOFF;                      /* put ghost at */
   xposn[c]=gdest_v[i]->x-XOFF;                      /* the vertex */

   if(revover && !energized[i])
   {
     gdx=xposn[0]-xposn[c];                          /* dist to pacman */
     gdy=yposn[0]-yposn[c];
   }
   else                                        /* Make them seek home vertices. */
   {
     gdx=minmaxx[i]-xposn[c];
     gdy=minmaxy[i]-yposn[c];
   }

   numchoices=0;
   choice=-1;
   temp_pt=gv[i];
   gvi=gv[i]=gdest_v[i];                  /* We have arrived at this vertex. */
   gdest_v[i]=NULL;
   if (stay[i]>0)
   {
    --stay[i];
/*    if(!stay[i]) changespeed(i,basespeed[i]); */
    choice=(gdiri+2)&3;
   }
   else if (energized[i]<0)
   {
    xposn[c]=gvi->x-XOFF;
    yposn[c]=gvi->y-YOFF;
    if (energized[i]==-2)
    {
     if(gvi->next[gdiri]) choice=gdiri;
     DeEnergize(i,2);
    }
    else if(gvi!=ghost_start)
    {
      if (gvi->backtrack!=((gdiri+2)&3))
        choice = gvi->backtrack;
                                                       /* Prevent turnarounds */
      else for(look=3;choice<0;look++)
      {
        consider = (gdiri+look)&3;

       if(gvi->next[consider]) choice = consider;
      }
    }
    else        /* we have now arrived at the originating "ghost_start" */
    {
     choice=(i+2)&3;      /* This disperces ghosts according to spr num */
     if(!gvi->next[choice]) choice=(gdiri+2)&3;
     energized[i]=-2;        /* Make note to de-energize on next vertex */
    }
   }

   if(gvi==ghost_start && choice<0) /* Jail break ASAP for ghosts */
   {
    pref2=(gdiri+1)&3; if(gvi->next[pref2]) choice=pref2;
    else {pref2=(gdiri+3)&3; if(gvi->next[pref2]) choice=pref2;}
    if(choice<0)      /* This one re-releases ghost after returning home */
    {pref2=(gdiri+2)&3; if(gvi->next[pref2]) choice=pref2;}
   }   
   for(tempchoice=-1,look=3;look<6 && choice<0;look++)
                                          /* If ghost can "see" PACMAN, */
   {                                      /* he is likely to follow him */
    consider=(gdiri+look)&3;
    if (gvi->next[consider] && gvi->next[consider]!=center_box
     && !(gvi->code[consider]&(FORBID|GHOST_1WAY)))
    {
     ++numchoices;
     tempchoice=consider;
     if (gvi->next[consider]==pdest_v &&
         gint[i]!=STUPID && revover && gvi!=top_box &&
       (!(gvi->code[consider]&(TUNNEL|FORBID|GHOST_1WAY)) ||
         (gvi->code[consider]&(TUNNEL) && gvi==pmv)) )
          choice=consider;
    }
   }
   if(!numchoices && choice<0) choice=(gdiri+2)&3;
                                             /* If dead end,turn around */
   else if (gvi==top_box && temp_pt==center_box && choice<0)
   {
     choice = (gdiri+3)&3; /* When ghost first emerges from box */
     inbox[i]=0;
     if (energized[i]==0) changespeed(i,basespeed[i]);
     else changespeed(i,bluespeed);
   }
   else if(numchoices==1 && choice<0) choice=tempchoice;
   /* Ghost must turn left when coming from center box. */
   else if(!revover && !home[i] && choice<0)
   {
     consider = rnd(numchoices);
     pref2=-1;
     for(look=3;look<10 && choice<0;look++)
     {
       pref=(look+gdiri)&3;
       if(gvi->next[pref] && gvi->next[pref]!=center_box 
          && !(gvi->code[pref]&(FORBID|GHOST_1WAY)))
          if(++pref2>=consider) choice=pref;
     }
   }
   else if(choice<0)   /* Use general dir of pacman & gint[i] for choice */
   {
    /* 1st pref is best, 2nd is next, 3rd next, etc. */
    /* 1st pref = "pref", 2nd = pref2, 3rd = pref2+2, 4th = pref+2 */
    /* pref3 takes place from default if pref and pref2 are not possible */
    /* and it is therefore not stored in any variable "pref3" */
    /* Each ghost modifies the normal prefs for his intelligence */


    /* RED ghost has best prefs - (1st then 2nd) */

    xgty=(ABS(gdx) > ABS(gdy));
    if (gint[i]==BLUE || (gint[i]==PINK && gdx < (xfactor<<3)
       && gdy < (yfactor<<3) ) ) xgty=!xgty;

    if (!revover) xgty=(ABS(gdx) > ABS(gdy));
    /* 2nd pref then 1st pref */

    /* Preferred directions */
    pref=(!xgty) ? ((gdy>0)?2:0) : ((gdx>0)?1:3);
    pref2=(xgty) ? ((gdy>0)?2:0) : ((gdx>0)?1:3);
    pref3=(pref2+2)&3;
    
    if (!revover) pref3=(pref+2)&3;

    if (gint[i]==STUPID && revover)
    {
     pref3=(pref+2)&3;  /* 4th best move */
     pref =pref2;      /* 2nd best move */
     pref2=(pref2+2)&3; /* 3rd best move */
    }

#define checkpref(pref)  (gvi->next[pref] \
                      &&  gvi->next[pref]!=temp_pt  \
                      &&!(gvi->code[pref]&(FORBID|GHOST_1WAY)) \
                      &&  gvi->next[pref]!=center_box)
    
         if checkpref(pref ) choice=pref;
    else if checkpref(pref2) choice=pref2;
    else if checkpref(pref3) choice=pref3;
    
    else
    {
     for(look=3;choice<0 && look<7;look++)/* Default moves if prefs fail */
     {
      consider=(gdiri+look)&3;
      if (gvi->next[consider] && gvi->next[consider]!=center_box &&
         !(gvi->code[consider]&(TUNNEL|FORBID|GHOST_1WAY)))
            choice=consider;
     }
    }                                    /* We should now have a choice */
   }        /* This block executed if PM is not on a connecting vertex. */
   gdest_v[i]=gvi->next[choice];
   gdir_code[i]=choice;
   gx[i]=(choice==1)-(choice==3);
   gy[i]=(choice==2)-(choice==0);

   mdist = ABS(mdistx+mdisty);
   mdisty = (gy[i]==0)?0:((gy[i]>0)?mdist:-mdist);
   mdistx = (gx[i]==0)?0:((gx[i]>0)?mdist:-mdist);
   
   if (gvi->code[gdir_code[i]]&TUNNEL)
   {
     wasintunnel[i]=1;
     if(energized[i]>=0) /* Eyes don't slow down in tunnels! */
        changespeed(i,(energized[i])?bluetunnel:tunnelspeed);
   }
   else if (wasintunnel[i])
   {
     wasintunnel[i]=0;
     changespeed(i,(energized[i])?((energized[i]<0) ? eyespeed:bluespeed)
                                 :basespeed[i]); 

      /* might need to add condition for eyes going thru tunnel **/
   }
     
  }  /* end at_vertex */  

  xposn[c]+=mdistx;
  yposn[c]+=mdisty;
  WrapSprite(c,mdistx,mdisty);

  if(revtoggle[i] && !inbox[i] && energized[i]>=0)
  {
    if (revtoggle[i] == TOGGLE_ENERG || num_energized == 0)
    {
      revtoggle[i] = 0;
      temp_pt=gv[i];
      gv[i]=gdest_v[i];
      gdest_v[i]=temp_pt;
      gx[i]=-gx[i];
      gy[i]=-gy[i];
      gdir_code[i]=(gdir_code[i]+2)&3;
    }
  }

  gdx=xposn[0]-xposn[c];
  gdy=yposn[0]-yposn[c];

  if (ABS(gdx)+ABS(gdy)<COLLDIST && energized[i]>=0)
    gotem=(energized[i]>0)?(i+1):(-1); /*>0:monspr#/-1:dead */  

  if (energized[i]>0)
  {
   if(!(--energized[i]))
   {
     DeEnergize(i,1);
   }
   else if(energized[i]<161 && !(energized[i]&15))
     BlinkMonster(i,(energized[i]&16)==0);
   ChangeSprite(svp,&sprite[c],(void *)&ghosts[i*6+4]);
  }
  else if(!energized[i])
    ChangeSprite(svp,&sprite[c],(void *)&ghosts[i*6+gdir_code[i]]);
  else
    ChangeSprite(svp,&sprite[c],(void *)&eyes[i*4+gdir_code[i]]);
 }
 return gotem;
}

BlinkEnergizer(int r)
{
  if(r) SetRGB4(viewPort,ENERGCOLOR,                  /* Two times/sec */
                       colorvalue[ENERGCOLOR][R_RGB],
                       colorvalue[ENERGCOLOR][G_RGB],
                       colorvalue[ENERGCOLOR][B_RGB]);
  else SetRGB4(viewPort,ENERGCOLOR,
                       colorvalue[PATHCOLOR][R_RGB],
                       colorvalue[PATHCOLOR][G_RGB],
                       colorvalue[PATHCOLOR][B_RGB]);

  return 0;
}
