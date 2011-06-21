/*** PTREE.C - This module determines the path that leads to ghostbox. ***/
/*
/* The infamous shortest path algorithm.
/*************************************************************************/

#include <exec/types.h>            /* Amiga specific variable type declarations */
#include <exec/memory.h>            /* Used for allocation of CHIP RAM. */

#define global extern

#include "pacdef.h"
#include "pacvar.h"

ShortTree(vertex_pt mst)
{
 short j;
 short countnodes;
 short top,index;
 short smalldist,consider_dist;
 short takenote;
 
 countnodes=CountNodes(v_head)+6; /* six for the ghost box */

        /* Same as declaring vertex_pt heap[countnodes], but is dynamic */
 if((heap=(vertex_pt *)AllocMem(sizeof(vertex_pt *)*countnodes,MEMF_CHIP))==0)
 {
  printf("Error requesting memory for minimum spanning tree's heap!\n");
  FreeMemory();
  exit(100);
 }

 theap=heap;
 MakeHeap();     /* Load and describe all vertices as unreachable */

 mst->curr_dist=0;           /* Make an assumption about where we start */

 for(top=countnodes-1;top>=0;top--)
 {
  smalldist=MAXDIST;
  for(index=0;index<=top;index++)            /* Find smallest curr_dist */
  {
   if(heap[index]->curr_dist < smalldist)
   {
    takenote=index;
    smalldist=heap[index]->curr_dist;
   }
  }
  if(smalldist<MAXDIST) for(j=0;j<4;j++) if(heap[takenote]->next[j])
  {
   consider_dist =heap[takenote]->curr_dist;
   consider_dist+=ABS(heap[takenote]->x-heap[takenote]->next[j]->x);
   consider_dist+=ABS(heap[takenote]->y-heap[takenote]->next[j]->y);

   if(consider_dist < heap[takenote]->next[j]->curr_dist)
   {
    heap[takenote]->next[j]->curr_dist=consider_dist;
    heap[takenote]->next[j]->backtrack=(j+2)&3;
   }
  }
  heap[takenote]=heap[top];
 }                                           /* End of the for top loop */
 FreeMem(heap,sizeof(vertex_pt *)*countnodes);
 
 return 0;
}

CountNodes(vertex_pt cn)                                  /* Neat, huh? */
{
 short isnode=1;

 if(cn->left)  isnode+=CountNodes(cn->left);
 if(cn->right) isnode+=CountNodes(cn->right);

 return (int) isnode;
}

MakeHeap()
{
   short i;

   theap[0]=top_box;
   theap[1]=center_box;
   theap[2]=ghost_start;
   for(i=0;i<4;i++) theap[i+3]=box_side[i];
   for(i=0;i<6;i++) theap[i]->curr_dist=MAXDIST;
   theap+=6;

   LoadHeap(v_head);

   return 0;
}


LoadHeap(vertex_pt lh)
{
 if(lh->left)  LoadHeap(lh->left);
 *theap=lh;
 theap++;
 lh->curr_dist=MAXDIST;
 if(lh->right) LoadHeap(lh->right);

 return 0;
}
