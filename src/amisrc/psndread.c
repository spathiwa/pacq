/*** PSNDRREAD.C - This module controls the reading of the sounds files. */
/*
/*************************************************************************/

#include "iff.h"
#include "play.h"

#define skip1(file) lseek(file, 1L, 1)
#define skip(file, amt) { lseek(file, (long) amt, 1);   \
                          if (isodd(amt)) skip1(file);}

extern void closesound();

int getIFF(char *filename, struct svx_info *info);
void freeInfo(struct svx_info *info);
void setpath(char *path);

void getCAT(int file);
void getLIST(int file);
void NotIFF(int file);
void getFORM(int file, struct svx_info *info);
void get8SVX(int file,ULONG size, struct svx_info *info);
ULONG getCk(int file, struct svx_info *info);
ULONG getID(int file);
void IFFRet (ULONG ckID,UBYTE *ckData,ULONG ckSize,struct svx_info *info);
void printVHDR(Voice8Header *vhdr);
void readerror (char *err, struct svx_info *info);
void getCHAN(int *channel);
void getBODY(char *body);
void addANNO(char *anote,struct svx_info *info,int len);
void printANNO(struct svx_info *info);
void freeANNO(struct anno *anno);
void freepath();
void printerror (char *err);

static char *defpathname;

void freepath()
{
  short bytes;

  if (defpathname)
  {
    bytes=strlen(defpathname+1);
    FreeMem (defpathname, bytes);
  }
}

void setpath(char *path)
{
  short bytes;

  freepath();

  bytes=strlen(path+1);
  defpathname = (char *) AllocMem(bytes,0);

  msgf("Path len %d\n",bytes-1);

  strcpy(defpathname, path);

  msgf("New path: %s\n",defpathname);
}

int getIFF(char *filename, struct svx_info *info)
{
  int file;
  short bytes;
  char *fullname;

  msg("Going to malloc for fullname\n");

  bytes= strlen(defpathname)+strlen(filename)+1;

  fullname = (char *) AllocMem(bytes,0);
  /* *fullname = 0; */

  msgf("Path: %s\n",defpathname);
  msgf("Filename: %s\n",filename);
  
  strcpy(fullname, defpathname);
  strcat(fullname, filename);
  
  msgf("Loading %s...\n",fullname);

  if ((file = open(fullname, O_RDONLY,0)) == -1)
  {
    printerror("Can't open file");
    FreeMem(fullname,bytes);
    return (-1);
  }
    
  FreeMem(fullname,bytes);
  
  switch (getID(file))
  {
    case ID_FORM: getFORM(file,info); break;
    case ID_CAT:  getCAT(file);  break;
    case ID_LIST: getLIST(file); break;

    default: NotIFF(file);
  }
  close(file);
  return 0;
}

void getCAT(int file) {}

void getLIST(int file) {}

void NotIFF(int file) 
{
  printf("Not an IFF file\n");
}

void getFORM(int file, struct svx_info *info)
{
  ULONG size;

  size = getID(file);
  switch(getID(file))
  {
    case ID_8SVX: get8SVX(file,size, info);  break;
  
    default: skip(file,size);
  }
}

void get8SVX(int file,ULONG size, struct svx_info *info)
{
  while ((size-=getCk(file, info)) >0)
    ;
  if (size < 0)
    readerror ("Decrepit input in form 8SVX: size incorrect", info);
  if (isodd(size))
    skip1(file);
}

ULONG getCk(int file, struct svx_info *info)
{
  ULONG ckID, ckSize;
  UBYTE *ckData;

  ckID = getID(file);
  printf("");
  ckSize = getID(file);
  if ((ckData = (UBYTE *)AllocMem(ckSize+1,MEMF_CHIP)) == 0)
    readerror("Out of memory: getCk", info);
  else
  {
    read(file,ckData,ckSize);
    ckData [ckSize] = 0;
    IFFRet(ckID,ckData,ckSize,info);
    if (isodd(ckSize))
      skip1(file);
  }
  return ckSize;
}

ULONG getID(int file)
{
  ULONG data[1];

  read(file,(char *)data,4);
  return *data;
}

void IFFRet (ULONG ckID,UBYTE *ckData,ULONG ckSize,struct svx_info *info)
{
  switch (ckID)
  {
    case ID_NAME:  info->name = ckData;
                   info->len.name = ckSize+1;
                   break;

    case ID_Copyright:  info->copy = ckData;
                        info->len.copy = ckSize+1;
                        break;

    case ID_AUTH:  info->auth = ckData;
                   info->len.auth = ckSize+1;
                   break;

    case ID_ANNO:  addANNO(ckData,info,ckSize+1);
                   break;

    case ID_CHAN:  info->chan = (int) *ckData;
                   FreeMem(ckData,ckSize+1);
                   break;

    case ID_VHDR:  info->vhdr = (Voice8Header *)ckData;
                   info->len.vhdr = ckSize+1;
                   break;

    case ID_BODY:  info->body = ckData;
                   info->len.body = ckSize+1;
                   break;

    default:       FreeMem(ckData,ckSize+1);
  } 
}

void getBODY(char *body)
{
}

void printVHDR(Voice8Header *vhdr)
{
  printf("One shot hi samples = %d\n",vhdr->oneShotSamps);
  printf("repeat samples = %d\n",vhdr->repeatSamps);
  printf("Samples per hi cycle = %d\n",vhdr->sampsPerHiCycle);
  printf("Samples per second = %d\n",vhdr->sampsPerSec);
  printf("ctOctaves = %d\n",vhdr->ctOctave);
  printf("sCompression = %d\n",vhdr->sCompression);
  printf("Volume = %d\n",vhdr->volume);
}

void printerror (char *err)
{
  printf("%s\n",err);
}

void readerror (char *err, struct svx_info *info)
{
  printf("ERROR: %s\n",err);
  FreeMemory();
}

void addANNO(char *anote,struct svx_info *info,int len)
{
  static struct anno *cur_anno;        
       
  if (info->anno == NULL)
  {
    if ((info->anno = (struct anno *)AllocMem(sizeof(struct anno),0)) == 0)
      readerror("Out of memory", info);
    
    cur_anno = info->anno;
  }
  else
  {
    if ((cur_anno->next = (struct anno *)AllocMem(sizeof(struct anno),0))
               == 0)
      readerror("Out of memory", info);

    cur_anno = cur_anno->next; 
  }
  cur_anno->text = anote;
  cur_anno->next = NULL;
  cur_anno->len  = len;
}

void printANNO(struct svx_info *info)
{
  struct anno *anno;
  
  anno = info->anno;
  while (anno)
  {
    printf("%s\n",anno->text);
    anno = anno->next;
  }
}


void freeInfo(struct svx_info *info)
{
  if (info) 
  {    
    if (info->name) 
    {
      FreeMem(info->name,info->len.name);
      info->name = NULL;
    }
    if (info->auth) 
    {
      FreeMem(info->auth,info->len.auth);
      info->auth = NULL;
    }
    if (info->copy) 
    {
      FreeMem(info->copy,info->len.copy);
      info->copy = NULL;
    }
    if (info->body) 
    {
      FreeMem(info->body,info->len.body);
      info->body = NULL;
    }
    if (info->vhdr)
    {
      FreeMem(info->vhdr,info->len.vhdr);
      info->vhdr = NULL;
    }
    freeANNO(info->anno);
    info->anno = NULL;
  }
}

void freeANNO(struct anno *anno)
{
  struct anno *next;
       
  while(anno)
  {
    FreeMem(anno->text,anno->len);
    next = anno->next;
    FreeMem (anno,sizeof(struct anno));
    anno = next;
  }
}

/* main(int argc, char *argv[])
{
  char *filename = "";
  struct svx_info *info;
  
  if ((info = (struct svx_info *) AllocMem(sizeof(struct svx_info)
                                                 ,MEMF_CLEAR)) == 0)
    readerror("Out of memory", info);
  
  if (argc == 1)
  {
    printf("File name to load: ");
    scanf("%s",filename);
  }
  else
    filename = argv[1];

  getIFF(filename, info);

  printVHDR(info->vhdr);
  if (info->name) printf("\nName   : %s\n",info->name);  
  if (info->auth) printf("Author : %s\n",info->auth);  
  if (info->copy) printf("(c)    : %s\n",info->copy);  
  if (info->chan) printf("Channel: %d\n\n",info->chan);
  printANNO(info);

  freeInfo(info);
}

*/


