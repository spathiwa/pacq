/*** PPLAY.C - This module controls the playing of the sounds. ***********/

#include "iff.h"
#include "play.h"

#define NUM_SOUNDS      9
#define NUM_CHANNELS    4
#define NUM_EXTRA_IOA   1

#define DIE             0
#define DOTS            1
#define CREDIT          2
#define EXTRA           3
#define FRUIT           4
#define MONSTER         5
#define BACKGRND        6
#define EYES            7
#define ENERGIZE        8

#define FINISH          NUM_CHANNELS 

#define PRIORITY        27

UBYTE aMap[] = { 0x0f };

struct audio_info
{
  struct svx_info *info;
  char *name;
  /* struct IOAudio *ioa; */
  /* UBYTE aMap[4]; */
  /* int pri; */
  /* int dev_open; */
  /* short int playing; */
} ADinfo[NUM_SOUNDS];

/* struct IOAudio *ioa[NUM_SOUNDS]; */

struct IOAudio *ioa[NUM_CHANNELS+NUM_EXTRA_IOA];
int dev_open;

short int sndonchan[NUM_CHANNELS] = {-1, -1, -1, -1};

extern void printVHDR(Voice8Header *vhdr);
extern int getIFF(char *filename, struct svx_info *info);
extern void freeInfo(struct svx_info *info);
extern char nosound;

extern short speedindex;

void playsound(int sound, int cycles);
int killsound(int sound);
void killallsound();
void killchannel(int channel);
int soundplaying(int sound);

int loadsound(char *path);
void freesound();
int closesound();

void checkChannels();
void openPort(int channel);
int log2(int num);
int allocIOA();
void setWrite(int sound, int channel, int cycles);
int InitIOA();
void setFinish();
void playerror(char *mesg);

int allocIOA()
{
  int channel,error;
               
  for (channel=0;channel<NUM_CHANNELS+NUM_EXTRA_IOA;channel++)
  {
    if ((ioa[channel] = (struct IOAudio *)
                       AllocMem(sizeof(struct IOAudio)
                       , MEMF_PUBLIC | MEMF_CLEAR)) == 0)
     {
        printf ("ERROR: Out of memory - ioa\n");
       return 1;
     }
  }
   
   error = InitIOA();

   for (channel=1;channel<NUM_CHANNELS+NUM_EXTRA_IOA;channel++)
   {
     *ioa[channel] = *ioa[0]; 
     openPort(channel);
   }
   
   return error;
}

int initsound()
{
  int i, error;
  
  ADinfo[DIE].name           = "die";
  ADinfo[DOTS].name          = "dots";
  ADinfo[CREDIT].name        = "credit";
  ADinfo[EXTRA].name         = "extra";
  ADinfo[FRUIT].name         = "fruit";
  ADinfo[MONSTER].name       = "monster";
  ADinfo[BACKGRND].name      = "backgrnd";
  ADinfo[EYES].name          = "eyes";
  ADinfo[ENERGIZE].name      = "energize";

  error = allocIOA();

  if (!error) 
    for(i=0;i<NUM_SOUNDS;i++)
      if ((ADinfo[i].info = (struct svx_info *) AllocMem (sizeof
                             (struct svx_info), MEMF_CLEAR)) == 0)
      {
        printf("Out of memory for info struct\n");
       error = 1;
       break;
      }
      
  return error;
}

int loadsound(char *path)
{
  int i, error = FALSE;
  
  setpath (path);

  for (i=0;i<NUM_SOUNDS;i++)
  {
   /* printf("Loading PACSOUND#%d\n",i); */
   if (getIFF(ADinfo[i].name, ADinfo[i].info))
   {
     error = TRUE;
     break;
   }
  }
   return error;
}

void openPort(int channel)
{
         if ((ioa[channel]->ioa_Request.io_Message.mn_ReplyPort = 
                 (void *)CreatePort(ADinfo[channel].name,0)) == 0)
        playerror ("No more audio ports.");
}

int InitIOA()
{
  openPort(0);

  ioa[0]->ioa_Request.io_Message.mn_Node.ln_Pri = PRIORITY;
  ioa[0]->ioa_Data = aMap;
  ioa[0]->ioa_Length = sizeof(aMap);    

  if (OpenDevice(AUDIONAME, 0, ioa[0], 0))
  {
    printf("ERROR: Can't open sound device\n");
    return 1;
  }
  else 
    dev_open = TRUE;

  return 0;
}

void killchannel(int channel)
{
  if (nosound) return;

      ioa[FINISH]->ioa_Request.io_Command = ADCMD_FINISH;
      ioa[FINISH]->ioa_Request.io_Flags = 0;

    ioa[FINISH]->ioa_Request.io_Unit = (struct Unit *)(int) (1 << channel);
 
      BeginIO(ioa[FINISH]);
  
      WaitIO(ioa[channel]);
}

int killsound(int sound)
{
  int channel,ret=0;
       
  if (nosound) return 0;

  checkChannels();
       
  for (channel=0;channel<NUM_CHANNELS;channel++)
    if (sndonchan[channel]==sound)
      killchannel(channel),ret++;

  return ret;
}  

void killallsound()
{
  int channel;

  if(nosound) return;

  checkChannels();
  
  for (channel=0;channel<NUM_CHANNELS;channel++)
    if (sndonchan[channel] > -1)
      killchannel(channel);
}    

void setWrite(int sound, int channel, int cycles)
{
  /* printVHDR(ADinfo[sound].info->vhdr); */

  ioa[channel]->ioa_Request.io_Command = CMD_WRITE;
  ioa[channel]->ioa_Request.io_Flags = ADIOF_PERVOL | IOF_QUICK;
  
  if(sound==BACKGRND) ioa[channel]->ioa_Period=
  (UWORD) ( 0x369dc4L /(speedindex*250+ADinfo[sound].info->vhdr->sampsPerSec));
  else                ioa[channel]->ioa_Period =
  (UWORD) ( 0x369dc4L / ADinfo[sound].info->vhdr->sampsPerSec );

  ioa[channel]->ioa_Volume = ADinfo[sound].info->vhdr->volume / 1024;
  ioa[channel]->ioa_Data   = ADinfo[sound].info->body;
  ioa[channel]->ioa_Length = ADinfo[sound].info->vhdr->oneShotSamps;
  
  ioa[channel]->ioa_Request.io_Unit = (struct Unit *) (int) (1 << channel);

  ioa[channel]->ioa_Cycles = cycles;
}
  
int log2(int num)
{
  int base = 0;

  while (num > 1)
  {
    num >>= 1;
    base++;
  }
  
  return base;  
}

void checkChannels()
{
  int i;
  
  for (i=0;i<NUM_CHANNELS;i++)
  {
    if (sndonchan[i] > -1 && CheckIO(ioa[i]))
     sndonchan[i] = -1;
  }
}

int soundplaying(sound)
{
  int channel, retcode = 0;
  
  checkChannels();
  
  for (channel=0;channel<NUM_CHANNELS;channel++)
    if (sndonchan[channel] == sound)
      retcode++;

  return retcode;      
}

int getChannel()
{
  int i, channel = -1;

  for (i=0;i<NUM_CHANNELS;i++)
    if (sndonchan[i] < 0)
    {
      channel = i;
      break;
    }

  return channel;

}

void playsound(int sound, int cycles)
{
  int channel;

  if(nosound) return;

  checkChannels();  /* Updates sndonchan array */

  channel = getChannel();

  if (channel > -1)
  {
    setWrite(sound, channel, cycles);

    BeginIO(ioa[channel]);

    if (!CheckIO(ioa[channel])) sndonchan[channel] = sound;
  }
}

void freesound()
{
  int i;
  
  for (i=0;i<NUM_SOUNDS;i++)
    freeInfo(ADinfo[i].info);
}    
    
int closesound()
{
  int i;

  killallsound();

  for (i=0;i<NUM_SOUNDS;i++)
  {
      if (ADinfo[i].info)
      {
        freeInfo(ADinfo[i].info);
        FreeMem(ADinfo[i].info,sizeof(struct svx_info));
      }
  }   
  
  for (i=0;i<NUM_CHANNELS+NUM_EXTRA_IOA;i++)
  {
    if (ioa[i]->ioa_Request.io_Message.mn_ReplyPort)  
       DeletePort(ioa[i]->ioa_Request.io_Message.mn_ReplyPort);

    if (ioa[i]) FreeMem(ioa[i],sizeof(struct IOAudio));
  }

  if (dev_open) CloseDevice(ioa[0]);
  dev_open = FALSE;

  freepath();

  return 1;
  /* Signal for program to abort execution - it is used by onbreak call */
}

void playerror(char *mesg)
{
  printf("ERROR: %s\n",mesg);
  FreeMemory();
}

/*

void main(int argc, char *argv[])
{
  int sound = 0, mode = 0;
  char pathname[21];

  if(onbreak(&closesound)) printf("Could not set the breakpoint!\n");

  initsound();
  loadsound("pw:ASound/");

  while (sound >= 0)
  {
    printf("Sound and mode: (-1 to end) ");
    scanf("%d %d",&sound, &mode);
    if (sound >= 0 && sound < NUM_SOUNDS) 
    {
      switch (mode)
      {
        case 0: playsound(sound, 1);
                break;
                
        case 1: killsound(sound);
                break;
                
        case 2: killallsound();
                break;
                
        case 3: playsound(sound, 0);
                break;
                
        case 4: freesound();
                printf("New path name: ");
                scanf("%20s",pathname);
                loadsound(pathname);
                break;
               
        case 5: printf("Number of sound %d playing: %d\n",sound
                                                     ,soundplaying(sound));
                break;
                
        case 6: if (sound < NUM_CHANNELS) killchannel (sound);
                break;
      }
    }
  }

  closesound();
  printf("all DONE\n");
}

*/
