#include <stdio.h>
#include <exec/types.h>    /* Amiga specific variable type declarations */

#define global extern

#include "pacdef.h"
#include "pacvar.h"

char *basicmaze = "T:basicmaze",
     *basicvars = "T:basicvars";

char *basicmazedata = 
"16 0 0 25 0\n"
"0 0 0 -12 0\n"
"0 37 28 37 0\n"
"0 25 0 37 0\n"
"0 -12 28 -12 0\n"
"8 -12 28 37 28\n",

     *basicvarsdata =
"Cherry/Strawberry\n"
"6 6 7 4		      /* xfactor, yfactor, pathsize, & boxheight */\n"
"18 23 29 4		   /* gstsproff, pacsproff, scrsproff, numghosts */\n"
"000 AAA 000 FF8 04F F0F 004 FFF FFF	  /* Screen Colors - first eight */\n"
"0 1 3 2 1 0				  /* Ghost's intelligence levels */\n"
"0 8 2 2 0 7	    /* How long to stay in ghost box in # of turnarounds */\n"
"0 6 3 8 1 7		 /* How long to stay in ghost box if pacman dies */\n"
"1 0 0 0 1 1 /* TRUE/FALSE; do ghosts search home corner before first rev */\n"
"16 16 16 16 8 16					 /* Ghost speeds */\n"
"8  4  32  8 8  	    /* tunnel, box, eye, blue, bluetunnel speeds */\n"
"12620 /* How long stay blue */\n"
"400\n";


CreateBasicMaze()
{
   FILE *file;
   
   if(!(file = fopen(basicmaze,"w"))) return -1;
   else fprintf(file,basicmazedata),fclose(file);
   
   if(!(file = fopen(basicvars,"w"))) return -1;
   else fprintf(file,basicvarsdata),fclose(file);

   return 0;
}
