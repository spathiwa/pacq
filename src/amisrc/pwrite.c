/*** PWRITE.C ***/
#define global extern

#include <exec/types.h>
#include <libraries/arpbase.h>
#include <intuition/screens.h>
#include <stdio.h>
#include "pacdef.h"
#include "pacvar.h"

extern struct FileRequester *fileRequester;
extern struct ArpBase *ArpBase;
extern struct Screen *PlayScreen;
extern struct Window *PlayWindow;
extern struct RasInfo *rasInfo;

extern char filename[FCHARS+DSIZE+1];
extern char fr_name[FCHARS+1],lastgraph[FCHARS+1],fr_path[DSIZE+1];
extern char *title;

struct IntuiText registertext[]=
{
  { 1,0,JAM2,26, 3,0," I want to be a producer ($$$$) ",&registertext[1]},
  { 1,0,JAM2,26,11,0,"        1217 Shirley Way        ",&registertext[2]},
  { 1,0,JAM2,26,19,0,"    Bedford, Texas 76022 USA    ",0}
},
regnegtext=
{
   1,0,JAM2,6,3,0,"Okay!",0
},
regpostext=
{
   1,0,JAM2,6,3,0,"Mailing is complicated",0
};


FILE *f;

SaveGraph()
{
   short tx=rasInfo->RxOffset,ty=rasInfo->RyOffset;
   
   Scrollit(-tx,-ty);
   DisplaySprites(FALSE);

   fileRequester->fr_Hail="Save Graph as file";
   strcpy(fileRequester->fr_Dir,"pw:graph");
   strcpy(fileRequester->fr_File,lastgraph); /* Last maze loaded */
   LoadColors(&PlayScreen->ViewPort,FR_COLORS);

#if 1 || REGISTERED_USER
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
         WriteGraph(filename);
      }
   }
#else
   AutoRequest(PlayWindow,&registertext[0],&regpostext,&regnegtext,0L,0L,320,61);
#endif
   SetMazeColors();
   Scrollit(tx,ty);
   ScreenToFront(PlayScreen);
   DisplaySprites(sprites_on);
   ToggleScreenToFront(1);

   return 0;
}

WriteGraph(char *filename)
{
   if (!(f=fopen(filename,"w"))) return 0;

   WriteVert(v_head);

   fclose(f);
   
   return 0;
}

WriteVert(vertex_pt v0)
{
   vertex_pt v1;
   short i,j;
   short code0,code1;
   
   if (!v0) return 0;
   
   for (i=0;i<2;i++)
   {
      if (v1=v0->next[i])
      {
         j = (i+2) & 3;
         code0 = ((v0==gbv1) || (v0 == pac_start1)
            || (v0->code[i]&(FORBID|GHOST_1WAY)) || (v0->vcode&ENERGIZER));
         code1 = ((v1==gbv1) || (v1 == pac_start1)
            || (v1->code[j]&(FORBID|GHOST_1WAY)) || (v1->vcode&ENERGIZER));

         if (code0 || !code1)
           fprintf(f,"%d %d %d %d %d\n"
                    ,v0->code[i] | v0->vcode
                    ,v0->unitx,v0->unity,v1->unitx,v1->unity);
         if (code1)
           fprintf(f,"%d %d %d %d %d\n"
                   ,v1->code[j] | v1->vcode
                   ,v1->unitx,v1->unity,v0->unitx,v0->unity);
      }
   }
   
   WriteVert(v0->left);
   WriteVert(v0->right);

   return 0;
}
