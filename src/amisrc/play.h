#include <exec/types.h>
#include <exec/memory.h>
#include <devices/audio.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE  1
#define FALSE 0
   
#define msgf(str, var) while(0)
#define msg(str)          while(0)

struct anno
{
  char *text;
  struct anno *next;
  int len;
};

struct svx_len
{
  int vhdr;
  int name;
  int auth;
  int copy;
  int body;
};  

struct svx_info
{
  Voice8Header *vhdr;
  int chan;
  char *name;
  char *auth;
  char *copy;
  struct anno *anno;
  char *body;
  struct svx_len len;
};

struct sprt_len
{
  int name;
  int auth;
  int copy;
  int body;
  int spr_cols;
  int height;
};

struct spr_cols
{
   USHORT color[16];
};

struct sprt_info
{
  char *name;
  char *auth;
  char *copy;
  char *body;
  struct spr_cols *spr_cols;
  USHORT *height;
  struct sprt_len len;
};
