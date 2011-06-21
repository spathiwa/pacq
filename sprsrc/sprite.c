#include <stdio.h>            /* standard input/output library (printf) */
#include <exec/types.h>            /* Amiga specific variable type declarations */
#include <exec/memory.h>            /* Used for allocation of CHIP RAM. */
#include <dos.h>

typedef struct {UWORD packets[46];} sprite_type;

int CXBRK(void) { CleanUp();printf("\n");exit(0);return 0; }

struct mySpriteInfo
{
   UBYTE *filename;
   UWORD *data;
   int objcount,height,bytecount;
} msi;

#define SIZE sizeof(sprite_type)

main(int argc,char *argv[])
{
   FILE *outfile;
   register UWORD bit;
   register short i,j,t,write;

   for(write=-1,i=1;argv[i][0]=='-';i++)
   {
      for(j=1;argv[i][j];j++)
      switch(tolower(argv[i][j]))
      {
         case 'w': write=1;break;
         case 'r': write=0;break;
         default : break;
      }
      --argc;
   }

   if(argc>=2 && argc<=3 && !write)
   {
      msi.filename=argv[i];
      ReadSprites(&msi);
      if(argc==3) outfile=fopen(argv[i+1],"w");
      else outfile=stdout;
      fprintf(outfile,"animlen %d\n",1);
      fprintf(outfile,"numobjs %d\n",msi.objcount);

      for(i=0; i < (msi.bytecount>>1); i+=(SIZE>>1))
      {
         printf("{\n");
         for(t=i; t < i+(SIZE>>1) ; t+=2 )
         {
            for(bit=0x8000;bit;bit>>=1)
            {
               fprintf(outfile,"%c"," .xX"[((msi.data[t  ]&bit)!=0)+
                                 (((msi.data[t+1]&bit)!=0)<<1)] );
            }
            printf(outfile," ;\n");
         }
         printf(outfile,"};\n");
      }

      printf(outfile,"\n");
   }
   else if(argc==3 && write==1)
   {
      while(token=GetToken!=TK_EOF)
      {
         
      }
      msi.filename=argv[i+1]"Sprites.binary
   }
   else
   {
      printf("Usage: %s -r <binary_input_filename> [ascii_output_filename]\n"
             "       %s -w <ascii_input_filename> [binary_output_filename]\n"
             "Note:  Use > to output to file if no outfile specified.\n",
             argv[0],argv[0]);
   }


   CleanUp();

   return 0;
}

ReadSprites(struct mySpriteInfo *si)
{
   FILE *file=NULL;
   char buffer[80];
   int nobj=0;

   si->height=21;

   if(file=fopen(si->filename,"r"))                       /* everthing is ok */
   {
      fgets(buffer,80,file);
      sscanf(buffer,"%d\n",&si->objcount);
      si->bytecount=si->objcount*SIZE;
      if(si->data=(UWORD *)AllocMem(si->bytecount,0))
         nobj=fread((void *)si->data,SIZE,si->objcount,file);
   }
   else printf("%s: File not found error!\n\n",si->filename);
   fclose(file);

   if(nobj!=si->objcount)
      printf("Warning: Number of object found doesn't match header\n");

   return (file!=0);
}

CleanUp()
{
   if(msi.data)
   {
      FreeMem(msi.data,msi.bytecount);
      msi.data=NULL;
   }

   return 0;
}
