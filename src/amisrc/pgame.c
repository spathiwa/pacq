#include <exec/types.h>
#include <graphics/sprite.h>
#include <libraries/arpbase.h>                    /* For file requester */

#define global extern
#include "pacdef.h"
#include "pacvar.h"
#include "pacvar2.h"

PlayMaze()
{
   PrintStatus();             /* Prints the score and everything else */

   BlackOut(ALLBLACK);     /* Hide the reconstruction of the viewPort */
   WaitTOF(),WaitTOF();            /* Wait for the Blackout to finish */

   InitializeStarting();

   ScrollDisplay(SCROLLDEFINITELY);

   killallsound();

   button=0;joyx=0;joyy=0;     /* Initialize Joy Regs */

   JoyRead(CTRLR_RESET);

   while(!(joyx || joyy))
   {
      WaitTOF();
      JoyRead(CTRLR_READ);
   }

   playsound(SND_BACKGRND,0),playsound(SND_BACKGRND,0);

   SetAPen(rastport,PATHCOLOR);            /* We will be erasing dots */
   for(i=0;i<numghosts;i++) energized[i]=0;

   while(dead>=0 && numdots)
   {
      tx=xposn[0]+XOFF;ty=yposn[0]+YOFF;

      if(tx>=minx-1 && tx<=maxx+1 && ty>=miny-1 && ty<=maxy+1)
      {
         lookhere=ReadPixel(rastport,tx+x_dir,ty+y_dir);
         if(lookhere==DOTCOLOR)
         {
            atedot++;
            WritePixel(rastport,tx+x_dir,ty+y_dir);
            INCSCORE(10);
            if(--numdots==advspeed[speedindex])
            {
               speedindex++;
               if(killsound(SND_BACKGRND))
                  playsound(SND_BACKGRND,0),playsound(SND_BACKGRND,0);
            }
            playsound(SND_DOT,1);
         }
         else if(lookhere==ENERGCOLOR)
         {
            atedot=1;
            for(i=-1;i<2;i++) for(c=-1;c<2;c++)          /* Erase energizer */
               WritePixel(rastport,tx+(x_dir<<1)+c,ty+(y_dir<<1)+i);
            INCSCORE(50);
            if(--numdots==advspeed[speedindex])
            {
               speedindex++;
               if(killsound(SND_BACKGRND))
                  playsound(SND_BACKGRND,0),playsound(SND_BACKGRND,0);
            }
            playsound(SND_DOT,1);
            Energize();
         }
      }
      if((!joyx)^(!joyy)) joy_dir =(joyy)?(1+joyy):(2-joyx);
      if(x_dir || y_dir) dir_code=(y_dir)?(1+y_dir):(2-x_dir);
      dvx=xposn[0]+XOFF-pdest_v->x;
      dvy=yposn[0]+YOFF-pdest_v->y;
      old_dcode = dir_code;
      if((ABS(dvx)+ABS(dvy))<2 && joy_dir!=dir_code
          && pdest_v->next[joy_dir])
      {
         dir_code = joy_dir;
         dvx=0;dvy=0;
         xposn[0]=pdest_v->x-XOFF;
         yposn[0]=pdest_v->y-YOFF;
         at_vertex = TRUE;
      }
      else at_vertex=((!dvx && !dvy) || (x_dir==y_dir));

      if ((!joyx)^(!joyy))     /* if joy movement */
      {
         if (at_vertex)
         {
            if(joyx && pdest_v->next[joy_dir] 
              && !(pdest_v->code[joy_dir]&FORBID))
            {
               pmv=pdest_v;
               pdest_v=pdest_v->next[joy_dir];       /* Destination vertex */
               x_dir=joyx;                  /* Direction we are now headed */
               y_dir=0;
            } 
            else if(joyy && pdest_v->next[1+joyy] 
                   && !(pdest_v->code[1+joyy]&FORBID))
            {
               pmv=pdest_v;
               pdest_v=pdest_v->next[1+joyy];
               x_dir=0;
               y_dir=joyy;
            }
            else if ((!(pdest_v->next[joy_dir]) 
                || (pdest_v->code[joy_dir]&FORBID))
              && ((!(pdest_v->next[old_dcode])
               || (pdest_v->code[old_dcode]&FORBID))))
            /* If(at_vertex && no (allowable) next vertex) dead end;stop */
            {
               x_dir=0;
               y_dir=0;
            }
            else if (x_dir||y_dir) /* Joy in invalid dir on vertex */
            {
               pmv=pdest_v;  /* There is a path in current dir - follow it */
               pdest_v=pdest_v->next[old_dcode];
               dir_code = old_dcode;
            }
         }                          /* End of block for if at a vertex */
         else if (joyx && !(joyx+x_dir)) /* Reverses direction on edge */
         {
            x_dir=-x_dir;              /* Reverse current direction */
            tv=pdest_v;            /* Swap pdest_v with last vertex */
            pdest_v=pmv;
            pmv=tv;
         }
         else if (joyy && !(joyy+y_dir))       /* Reverses dir along y */
         {
            y_dir=-y_dir;              /* Reverse current direction */
            tv=pdest_v;            /* Swap pdest_v with last vertex */
            pdest_v=pmv;
            pmv=tv;
         }
      } /* End of block for if no button but movement on joystick */
      else if (at_vertex && (y_dir || x_dir))   /* move but no joymove */
      {
         if(x_dir && (!pdest_v->next[old_dcode]
           || (pdest_v->code[old_dcode]&FORBID)))
            x_dir=0;/* Stop @ Dead End */
         else if(y_dir && (!pdest_v->next[old_dcode] 
           || (pdest_v->code[old_dcode]&FORBID)))
            y_dir=0;
         else
         {
            pmv=pdest_v;                 /* Continue moving to next vertex */
               pdest_v=pdest_v->next[old_dcode];
         }
      }
      if (atedot<2)
      {
         xposn[0]+=x_dir;
         yposn[0]+=y_dir;
      }
      else atedot=0;

      WrapSprite(0,x_dir,y_dir);                /* Wrap around routine */
      spoff=((xposn[0]+yposn[0])>>1)&3;
      spoff=(spoff>2) ? (1+3*dir_code) : (spoff+3*dir_code);
      ChangeSprite(svp,&sprite[0],(short *)(pacmen+spoff));

      if ((dead=MoveGhosts())<0) {
         dead=-1;
         --lives;                        /* Kill a PACMAN */
         Die();
      }
      else {
         ScrollDisplay(SCROLLMAYBE);
         PrintScore();
      }
      if(dead>0) {
         killallsound();
         playsound(SND_MONSTER,1);
         playsound(SND_MONSTER,1);
         DeEnergize(dead-1,0);                          /* Eat a ghost! */
         playsound(SND_EYES,0);
      }
      JoyRead(CTRLR_READ);
   }                             /* End of main control loop */
   return 0;
}                                /* End of loop to decrement lives. */

BlinkMaze()
{
   return 0;
}