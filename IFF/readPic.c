struct Anim {
   UBYTE *planedata;
   ULONG reltime;
   struct Anim *next;
}

struct PicList {
   char *name;
   UWORD w,h;
   UBYTE nplanes;
   stuct Anim *pics;
   struct PicList *next;
}

struct PicFrame {
   struct ClientFrame *cf;
   struct PicList *currpic;
   struct PicList *lastpic;
}

getForm(GroupContext *parent)
{
   IFFP iffp;
   
   switch (parent->subtype) {
      case ID_ILBM: 
        iffp = getILBM(parent);
        break;
      case ID_ANIM:
        iffp = getAnim(parent);
        break;
      default:
        return (IFF_OKAY)  /* Ignore other FORMs */
   }
   
   return (iffp);
}

iffp getAnim (GroupContext *parent)
{
   PicFrame *pf;
   GroupContext formContext;
   
   pf = (PicFrame *)parent->clientFrame;
   if (pf->inAnim)
     return (IFF_OKAY);    /* Ignore nested Anims */
   pf->inAnim = 1;
   
   pf->currpic->next  
   = (struct PicList *)AllocMem(sizeof(struct PicList),MEMF_PUBLIC);
   pf->currpic = pf->currpic->next;
   if (!pf->currpic)
     return CLIENT_ERROR;

   iffp = OpenRGroup(parent, &formContext);
   CheckIFFP();
   
   do switch (iffp = GetFlChunkHdr(&formContext)) {
      /* If you want to check for chunks inside the ANIM, put
         cases here */
      ;
   } while (iffp >= IFF_OKAY);
   
   if (iffp != IFF_DONE) return (iffp);
   
   CloseRGroup(&formContext);
   DoSomethingWithAnim -----;
   return(iffp);
}

iffp getILBM(GroupContext *parent)
{
   PicFrame *pf;
   GroupContext formContext;
   
   pf = (PicFrame *)parent->clientFrame;
   
   iffp = OpenRGroup(parent, &formContext);
   CheckIFFP();
   
   if (pf->firstILBM) {
      pf->firstILBM=0;
      do switch (iffp = GetFChunkHdr(&formContext)) {
         case ID_BMHD: 
           iffp = getBMHD(&formContext);
           break;
         case ID_BODY:
           if (!pf->gotBMHD)
             iffp = BAD_FORM;
           else
             iffp = getBODY(&formContext);
           break;
         case ID_NAME:
           iffp = getName(&formContext);
           break;
         case END_MARK:
           if (!pf->gotBMHD)
             iffp = BAD_FORM;
           else
             iffp = IFF_DONE; 
           break ;
         case ID_ANHD:
       
         case ID_CMAP:
       
         case ID_DLTA:
      }
   }
      
   else {
      do switch (iffp = GetFChunkHdr(&formContext)) {
         case ID_ANHD:
           getANHD(&formContext);
           break;
      
         case ID_DLTA:

         case ID_CMAP:
       
         case ID_BODY:
      
         case ID_BMHD:

      }
   }
}

IFFP getANHD(GroupContext *context)
{
   IFFP iffp;
   PicFrame *pf;
   AnimHeader ANHD;
   
   pf = (PicFrame *)context->clientFrame;
   iffp = IFFReadBytes(&formContext, (BYTE *)&ANHD, sizeof(AnimHeader));
   
   pf->curranim->next = AllocMem(sizeof(struct Anim), MEMF_PUBLIC);
   pf->curranim = pf->curranim->next;
   
   pf->curranim->reltime = ANHD->reltime;
}     
   

IFFP getName(GroupContext *context)
{
   IFFP iffp;
   int szBuf;
   
   pf = (PicFrame *)context->clientFrame;
   if (pf->name)
     FreeMem(pf->name, strlen(pf->name));

   szBuf = ChunkMoreBytes(context)    
   pf->name = (BYTE *)AllocMem(szBuf, MEMF_PUBLIC);
   if (buf == NULL)
     iffp = CLIENT_ERROR;
   else
     iffp = IFFReadBytes(context, (BYTE *)pf->name, szBuf);
   CheckIFFP();
}

iffp getBMHD(formContext)
{
   BitMapHeader BMHD;
   PicFrame *pf;
   IFFP iffp;
   
   iffp = IFFReadBytes(&formContext, (BYTE *)&BMHD, sizeof(BitMapHeader));
   CheckIFF(); 
   
   pf = (PicFrame *)parent->clientFrame;
   pf->currpic->w = BMHD.w;
   pf->currpic->h = BMHD.h;
   pf->currpic->nPlanes = BMHD.nPlanes;
   pf->currpic->pics = (struct Anim *)AllocMem(sizeof(struct Anim),MEMF_PUBLIC);
   pf->gotBMHD = 1;
}

IFFP getBODY(&formContext)
{
   IFFP iffp;
   int szBuf;
   
   pf = (PicFrame *)parent->clientFrame;
   DeInterleave(&formContext);   /* Read BODY into pf->pics->bitmaps */
      
}
