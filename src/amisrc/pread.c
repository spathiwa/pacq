/*** PREAD.C ***/
/* ReadSprites, ReadMaze */

#include <stdio.h>            /* standard input/output library (printf) */
#include <exec/types.h>            /* Amiga specific variable type declarations */
#include <exec/memory.h>            /* Used for allocation of CHIP RAM. */
#include <intuition/intuitionbase.h>
#include <graphics/sprite.h>
#include <string.h>
#include <libraries/arpbase.h>             /* Used for the FileRequester */

#define global extern
#define MYDEBUG 0

#include "pacdef.h"
#include "pacvar.h"
#include "pcon.h"

extern USHORT scols1[MAXGHOSTS];             /* Sprite color values 0x(000-FFF) */
extern short gint[MAXGHOSTS];

vertex_pt get_vertex(short x0,short y0);           /* Functions for maze */
vertex_pt destroy_parentref(vertex_pt v0);
vertex_pt add_vertex(short x0,short y0); 
vertex_pt add_ghost_vertex(short x0,short y0); 

extern char *basicmaze,*basicvars,lastgraph[FCHARS+1]; /* Defined in pdefault.c */

ReadSprites() /* Reads "sprite.dat" into *sprite_data as level 2 memory */
{
 FILE *file;
 char buffer[80];

 if(file=fopen(sprite_dat,"r"))                              /* everthing is ok */
  {
   fgets(buffer,80,file);
   sscanf(buffer,"%d\n",&numsprites);      /* This is a global variable */
   if(sprite_data=(sprite_type *) AllocMem(sizeof(sprite_type)*numsprites,0))
    fread((void *)sprite_data,sizeof(sprite_type),numsprites,file);
  }
 else printf("%s: File not found error!\n\n",sprite_dat);
 fclose(file);
 return (int) (!file);
}

FreeMazeList(struct maze *first)
{
  struct maze *temp1;

  while(temp1=first)
  {
    first = first->next;
    if (temp1->headername) free(temp1->headername); 
    if (temp1->graphname) free(temp1->graphname); 
  }
   
  return 0;
}

ReadGame(char *gamename)
{
  FILE *file;
  char buffer1[80], buffer2[80], *c;
  struct maze *oldtmaze,*tmaze=NULL;

  if(game.gamename) free(game.gamename);
  game.gamename = NULL;
  FreeMazeList(game.firstmaze); game.firstmaze = NULL;
  FreeMazeList(game.nextmaze); game.nextmaze = NULL;

  if(!(file = fopen(gamename,"r")))
    return -1;

  fgets(buffer1,80,file);
  game.gamename = (char *) malloc(strlen(buffer1)+1);
  strcpy(game.gamename,buffer1);

  if (!feof(file))
  {
    fgets(buffer1,80,file);
    sscanf(buffer1,"%d",&game.numlives);
  }
  else return -1;

  if (!feof(file))
  {
    fgets(buffer1,80,file);
    sscanf(buffer1,"%d",&game.extraman);
  }
  else return -1;

  while(!feof(file))
  {
    fgets(buffer1,80,file);
    if(!feof(file))
    {
      fgets(buffer2,80,file);

      oldtmaze = tmaze;

      tmaze = (struct maze *) malloc(sizeof(struct maze));

      tmaze->next = NULL;

      tmaze->headername = (char *) malloc(strlen(buffer1)+1);
      strcpy(tmaze->headername,buffer1);
      if (c = strchr(tmaze->headername,' '))  *c = '\0';
      if (c = strchr(tmaze->headername,'\n')) *c = '\0';

      tmaze->graphname = (char *) malloc(strlen(buffer2)+1);
      strcpy(tmaze->graphname,buffer2);
      if (c = strchr(tmaze->graphname,' '))  *c = '\0';
      if (c = strchr(tmaze->graphname,'\n')) *c = '\0';

      if (!game.firstmaze) game.firstmaze = tmaze;
      else oldtmaze->next = tmaze;
      
      game.nextmaze = game.firstmaze;
      
      #if MYDEBUG
      printf("Read %s - %s\n",tmaze->headername,tmaze->graphname); */
      #endif
    }
  }

  fclose(file);

  return 0;
}

ReadHeader(char *mazename)       /* Create the maze on the pgm stack */
{
 FILE *file;
 char buffer[80];
 int k,c[9]; /* Temp color storage */
 int x1=0,y1=0,x2=0,y2=0;

 if((file=fopen(mazename,"r")) || (file=fopen(basicvars,"r")) )
 {
   fgets(maze_dat,20,file);              /* File name of the next maze. */
   for(i=0;i<strlen(maze_dat);i++)
   {
    if (maze_dat[i]==' ') maze_dat[i]='\0'; /* No spaces or CR's allowed */
    if (maze_dat[i]=='\n') maze_dat[i]='\0';
   }

   fgets(buffer,80,file);
   sscanf(buffer,"%d %d %d %d\n",&x1,&y1,&x2,&y2);
   xfactor=(short) x1;yfactor=(short) y1;pathsize=(short) x2;
   border = pathsize + BORDERSIZE;
   if (xfactor<2) xfactor = 2;
   if (yfactor<2) yfactor = 2;
   boxheight=(short) y2;

   fgets(buffer,80,file);
   sscanf(buffer,"%d %d %d %d\n",&x1,&y1,&x2,&y2);
   ghstsproff=(short) x1;pacsproff=(short) y1;scrsproff=(short) x2;
   numghosts=(short) y2;

   fgets(buffer,80,file);                              /* Screen colors */
   sscanf(buffer,"%x%x%x%x%x%x%x%x%x",c,c+1,c+2,c+3,c+4,c+5,c+6,c+7,c+8);
   for(i=9;i<32;i++) for(k=0;k<3;k++) colorvalue[i][k]=0; /* Black out */
   for(i=0;i<9;i++)
   {
     colortable[i]=c[i];
     colorvalue[i][R_RGB]=((c[i]&0xF00) >>8);
     colorvalue[i][G_RGB]=((c[i]&0x0F0) >>4);
     colorvalue[i][B_RGB]=((c[i]&0x00F) >>0);
   }

   fgets(buffer,80,file);                       /* Ghost's intelligence */
   sscanf(buffer,"%d %d %d %d %d %d",c,c+1,c+2,c+3,c+4,c+5);
   for(i=0;i<numghosts;i++)
     gint[i]=c[i];

   fgets(buffer,80,file);     /* How long to initially stay in ghost box */
   sscanf(buffer,"%d %d %d %d %d %d",c,c+1,c+2,c+3,c+4,c+5);
   for(i=0;i<numghosts;i++) stay[i]=(c[i]<<1)+(c[i]!=0);

   fgets(buffer,80,file);  /* How long to stay in ghost box after pacdie */
   sscanf(buffer,"%d %d %d %d %d %d",c,c+1,c+2,c+3,c+4,c+5);
   for(i=0;i<numghosts;i++) stayaftdie[i]=(c[i]<<1)+(c[i]!=0);

   fgets(buffer,80,file);  /* 0/1 - Random movement or seek home corner? */
   sscanf(buffer,"%d %d %d %d %d %d",c,c+1,c+2,c+3,c+4,c+5);
   for(i=0;i<numghosts;i++) home[i]=(c[i]!=0);

   fgets(buffer,80,file);                            /* Ghost base speed */
   sscanf(buffer,"%d %d %d %d %d %d",c,c+1,c+2,c+3,c+4,c+5);
   for(i=0;i<numghosts;i++)
     basespeed[i]=c[i];

   fgets(buffer,80,file);                          /* Other Ghost speeds */
   sscanf(buffer,"%d %d %d %d %d",c,c+1,c+2,c+3,c+4);
   tunnelspeed = c[0];
   boxspeed = c[1];
   eyespeed = c[2];
   bluespeed = c[3];
   bluetunnel = c[4];

   fgets(buffer,80,file);                             /* energizer time */
   sscanf(buffer,"%d",c);
   energtime=*c;

   fgets(buffer,80,file);/* Number of clck ticks before reversing ghosts */
   sscanf(buffer,"%d",c);                         /* for the first time */
   firstrev=*c;
 }

 if(file)
 {
   fclose(file);
   file = NULL;
   mazestatus|=VARS_LOADED; /* Loaded */
   mazestatus&=(~VARS_CHANGED); /* No change yet */
   return 0;
 }
 else return 1;
}

/* read in vertices */
ReadGraph(char *mazename)
{
 FILE *file;
 char buffer[80];
 char *ferror;

 short sx1,sy1,sx2,sy2;
 vertex_pt v1,v2;
 int x1=0,y1=0,x2=0,y2=0;

 /* Save the name of last graph loaded */
 ferror=strchr(mazename,'/')+1;
 if(!ferror) ferror=strchr(mazename,':')+1;
 if(ferror) strncpy(lastgraph,ferror,FCHARS);

 maxx=-32000;maxy=-32000;minx=32000;miny=32000;
 FreeMaze();
 offsetx = offsety = 0; /* Needed! */

 if((file=fopen(mazename,"r")) || (file = fopen(basicmaze,"r")) )
  {
   do {
       ferror=fgets(buffer,80,file);
       sscanf(buffer,"%d %d %d %d %d\n",&ecode,&x1,&y1,&x2,&y2);
       sx1=(short) (x1*xfactor);sy1=(short) (y1*yfactor);
       sx2=(short) (x2*xfactor);sy2=(short) (y2*yfactor);  /* Proportion */
       if (/*sx1>=0 &&*/ ferror)
       {
        boundaries(sx1,sy1);                            /* find min/max */
        boundaries(sx2,sy2);
        v1=add_vertex(sx1,sy1);
        v2=add_vertex(sx2,sy2);
        if(ecode & ENERGIZER) v1->vcode|=ENERGIZER;
        if(ecode & PACSTART) {pac_start1=v1;pac_start2=v2;}
        if(ecode & GHOSTSTART) MakeGhostBox(gbv1=v1,gbv2=v2);
        else
        {
         connectum(v1,v2);
          ecode&=~(FORBID|GHOST_1WAY|PACSTART); /*Dired codes put only v1*/
         connectum(v2,v1);
        }
       }
      }
   while ( /*sx1>=0 &&*/ ferror );
   
/* check vertices and set up basic vars */
   
   if(!ghost_start) ghost_start=v_head;
   if(!pac_start1)
   {
    pac_start1=v_head;
    for(i=0;i<NUMEDGES;i++) if (pac_start1->next[i])
                    pac_start2=pac_start1->next[i];
   }
   AdjustMazeVars(MINMAX);

   for(i=0;i<=numghosts;i++)
   {
     minmaxx[i]=(gint[i]&1)?minx:maxx; /* Red & Blue to maxx */
     minmaxy[i]=(gint[i]&2)?maxy:miny; /* Pink & Red to miny */
   }
  }

 if(file)
 {
   fclose(file);
   file = NULL;
   mazestatus|=GRAPH_LOADED;
   mazestatus&=(~GRAPH_CHANGED); /* No change yet */
   return 0;
 }
 else return 1;
}

ReadMaze()
{
  int ferror;
  char header[50] = "pw:vars/", graph[50] = "pw:graph/";

  strcat(header,game.nextmaze->headername);
  strcat(graph ,game.nextmaze->graphname);
  
  ferror=(ReadHeader(header)) ||
         (ReadGraph(graph));

  if (!(game.nextmaze = game.nextmaze->next))
    game.nextmaze = game.firstmaze;

  ResizeMaze(v_head);

  ShortTree(ghost_start);

  return ferror;
}

MakeGhostBox(vertex_pt vp1,vertex_pt vp2)
{
 short adx,ady;
 vertex_pt gv1,gv2,gv3;

 if (!(vp1 && vp2)) return 0;

 DelGhostBox();

 adx=vp2->x - vp1->x;
 ady=vp2->y - vp1->y;
 if(adx) adx= (adx>0)?1:-1;
 if(ady) ady= (ady>0)?1:-1;

 ecode = 0;
 gv1=add_ghost_vertex((short)((vp1->x + vp2->x)/2),
                      (short)((vp1->y + vp2->y)/2));

 connectum(gv1,vp1);
 ecode = GHOSTSTART;
 connectum(vp1,gv1);
 ecode = 0;
 connectum(gv1,vp2);               /* Connected the ghost emerging point */
 connectum(vp2,gv1);
 top_box = gv1;         /* This is where the red ghost usually starts... */

 gv3=add_ghost_vertex((short) (gv1->x-ady*((pathsize*2)+2)),
                      (short) (gv1->y+adx*((pathsize*2)+2)));
 ecode=FORBID;
 connectum(gv1,gv3);        /* Center of the ghost box is now connected */
 ecode=0;
 connectum(gv3,gv1);
 center_box=gv3;

 gv2=add_ghost_vertex((short) (gv3->x-ady*boxheight),
                      (short) (gv3->y+adx*boxheight));
 connectum(gv2,gv3);  /* Bottom-center of the ghost box is now connected */
 connectum(gv3,gv2);
 ghost_start=gv2;

 gv1=add_ghost_vertex((short) (gv2->x+adx*((pathsize<<1)-2)),
                      (short) (gv2->y+ady*((pathsize<<1)-2)));
 connectum(gv1,gv2);                         /* Side 1 of the ghost box */
 connectum(gv2,gv1);
 box_side[0]=gv1;

 gv3=add_ghost_vertex((short) (gv1->x+ady*boxheight),
                      (short) (gv1->y-adx*boxheight));
 connectum(gv1,gv3);                       /* Side 1.5 of the ghost box */
 connectum(gv3,gv1);
 box_side[1]=gv3;

 gv1=add_ghost_vertex((short) (gv2->x-adx*((pathsize<<1)-2)),
                      (short) (gv2->y-ady*((pathsize<<1)-2)));
 connectum(gv1,gv2);                         /* Side 2 of the ghost box */
 connectum(gv2,gv1);
 box_side[2]=gv1;

 gv3=add_ghost_vertex((short) (gv1->x+ady*boxheight),
                      (short) (gv1->y-adx*boxheight));
 connectum(gv1,gv3);                       /* Side 2.5 of the ghost box */
 connectum(gv3,gv1);
 box_side[3]=gv3;

 return 0;
}

DelGhostBox()
{
   short count=0, cb_dir,i;
   vertex_pt v1,v2;

   if (!center_box || !top_box || !(gbv1 && gbv2)) return 0;

   /* Check top_box for the dir of center_box */
   for (i=0;i<4;i++)
   {
     if (top_box->next[i]) count++;
     if (top_box->next[i] == center_box) cb_dir = i;
   }

   if (count<4) /* Should always occur */
   {
      v1 = top_box->next[(cb_dir+1)&3]; /* gbv1 & gbv2... */
      v2 = top_box->next[(cb_dir+3)&3];

      DeleteGhostVertex(top_box); top_box = NULL;

      ecode = v1->code[(cb_dir+3)&3]&~(GHOSTSTART);
      connectum(v1,v2);

      ecode = v2->code[(cb_dir+1)&3]&~(GHOSTSTART);
      connectum(v2,v1);
   }

   DeleteGhostVertex(center_box); center_box = NULL;
   DeleteGhostVertex(ghost_start); ghost_start = NULL;

   for(i=0;i<4;i++)
   {
      DeleteGhostVertex(box_side[i]);
      box_side[i] = NULL;
   }

   return 0;
}

vertex_pt add_ghost_vertex(short x0,short y0)
{
   vertex_pt v0; /* Ghost vertices not in binary tree */

   boundaries(x0,y0);

   v0=(vertex_pt) AllocMem(sizeof(struct vertex),MEMF_PUBLIC|MEMF_CLEAR);
   v0->x = x0;
   v0->y = y0;
   v0->unitx = (short)(x0-offsetx)/xfactor;
   v0->unity = (short)(y0-offsety)/yfactor;
   v0->backtrack=-1;
   v0->left=v0->right=NULL;

   v0->vcode=GHOST_BOX;

   return (vertex_pt) v0;
}

boundaries(short xp,short yp)
{
 if (xp>maxx) maxx=xp;
 if (xp<minx) minx=xp;
 if (yp>maxy) maxy=yp;
 if (yp<miny) miny=yp;

 return 0;
}

remap_maze(vertex_pt v0)
{
 if (!v0) return 0;
 remap_maze(v0->left);
 remap_maze(v0->right);

 v0->x = v0->x + offsetx;
 v0->y = v0->y + offsety;

 return 0;
}

connectum(vertex_pt vtx1,vertex_pt vtx2)          /* link vtx2 to vtx1 */
{
 char direction;                       /* 0=up, 1=right, 2=down, 3=left */
 short dx,dy;
 
 dx = vtx2->x - vtx1->x;
 dy = vtx2->y - vtx1->y;
 if ((dx && dy) || !(dx || dy))
  {
   #if MYDEBUG
   printf("Invalid connecting vertices! - ");
   printf("(%3d,%3d),(%3d,%3d)\n",vtx1->x,vtx1->y,vtx2->x,vtx2->y);
   #endif
  }
 else
  {
   direction = (char) ((dx>0)+3*(dx<0)+2*(dy>0)+2*((ecode&TUNNEL)!=0))%4;
   if (!(ecode & NO_MOD))
      vtx1->code[direction]|=
         ecode&(FORBID|GHOST_1WAY|DOTS|TUNNEL|GHOSTSTART|PACSTART);
   vtx1->next[direction] = vtx2;
  }
 return 0;
}

vertex_pt add_vertex(short x0,short y0)      /* Add a vertex to the maze */
{
 vertex_pt v0;

 if((v0=get_vertex(x0,y0))==NULL)
  {
   v0=(vertex_pt) AllocMem(sizeof(struct vertex),MEMF_PUBLIC|MEMF_CLEAR);
   v0->x = x0;
   v0->y = y0;
   v0->unitx = (short)(x0-offsetx)/xfactor;
   v0->unity = (short)(y0-offsety)/yfactor;
   v0->backtrack=-1;
   v0->left=NULL;
   v0->right=NULL;
   insert(v0);                           /* Attach binary tree pointers */
  }
 return (vertex_pt) v0;
}

vertex_pt get_vertex(short x0,short y0)              /* Search for vertex x0,y0 */
{                                             /* returns NULL if !found */
 vertex_pt temp;
 temp=v_head;
 while (temp && (temp->x != x0 || temp->y != y0))
  {
   if ((x0+y0) > (temp->x + temp->y)) temp=temp->right;
   else temp=temp->left;
  }
 return (!temp || (in_con&&(temp->vcode&GHOST_BOX)))? NULL:(vertex_pt)temp;
}

vertex_pt destroy_parentref(vertex_pt v0) /* Parent || NULL if unsucc */
{
   vertex_pt parent=NULL,temp=v_head;
   if(v0 == v_head) v_head = NULL; /* reference is "v_head" */
   else
   {
      while (temp && (temp->x != v0->x || temp->y != v0->y))
      {
         parent=temp;
         if ((v0->x + v0->y) > (temp->x + temp->y)) temp=temp->right;
         else temp=temp->left;
      }
      if (temp)
      {
         if ((v0->x + v0->y) > (parent->x + parent->y)) parent->right=NULL;
         else parent->left=NULL;
      }
   }

   return (temp)?parent:NULL;
}

insert(vertex_pt v0)                 /* Connects binary tree ptrs to v0 */
{
 vertex_pt t2,temp;

 if (!v_head) v_head=v0;
 else
  {
   temp=v_head;
   while (temp)
    {
     t2=temp;
     if ((v0->y + v0->x) > (temp->y + temp->x)) temp=temp->right;
     else temp=temp->left;
    }
   if (t2 == v0) return 0; /* Do not insert twice! */
   else if ((v0->y + v0->x) > (t2->y + t2->x)) t2->right=v0;
   else t2->left=v0;
  }
 return 0;
}

AdjustMazeVars(short minmax)
/* After you figure min/max call this to figure others */
{
   width = MAX(WIDTH, maxx-minx+((pathsize+BORDERSIZE)<<1));
   if(in_con) width+=(xfactor<<2);
   width = (width+15)&~15;   /* Rounds width up to nearest multiple of 8 */
   height= MAX(dheight,maxy-miny+((pathsize+BORDERSIZE)<<1));
   if(in_con) height+=(yfactor<<2);
   superx= (width !=WIDTH );
   supery= (height!=dheight);
   super = (superx || supery);
   offsetx=((width -(maxx-minx))/2)-minx;
   offsety=((height-(maxy-miny))/2)-miny;

   if(minmax==MINMAX)
   {
      minx+=offsetx ; maxx+=offsetx ;
      miny+=offsety ; maxy+=offsety ;
   }

   #if MYDEBUG
   printf("offsetx: %d offsety: %d minx: %d maxx:%d\n"
          "miny: %d maxy:%d\n",offsetx,offsety,minx,maxx,miny,maxy);
   #endif

   remap_maze(v_head);            /* add offsets to each x and y coords */

   return 0;
}

DeleteVertex(vertex_pt v0)
{
   short i,j;
   
   if (!v0) return 0;

   destroy_parentref(v0);

   for(i=0;i<4;i++)
   {
      j=(i+2)&3;
      if(v0->next[i])
      {
         v0->next[i]->next[j]=NULL; /* Delete reference to itself */
         v0->next[i]->code[j]=0;
         /* CheckRedundant(v0->next[i]); */
      }
   }
   if(v0->right) insert(v0->right);
   if(v0->left)  insert(v0->left);

   FreeMem(v0,sizeof(struct vertex));

   return 0;
}

DeleteGhostVertex(vertex_pt v0)
{
   short i,j;

   for(i=0;i<4;i++)
   {
      j=(i+2)&3;
      if(v0->next[i])
      {
         v0->next[i]->next[j]=NULL; /* Delete reference to itself */
         v0->next[i]->code[j]=0;
         /* CheckRedundant(v0->next[i]); */
      }
   }
   
   FreeMem(v0,sizeof(struct vertex));

   return 0;
}
