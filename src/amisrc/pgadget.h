#define FLAGGADG 0x8000
#define MAXSTRLEN 16

#define NUMGADS         23
#define NUM_STRIPS      3

struct IntuiText gadgetintui[NUMGADS];
struct Border gadgetborder[2][MAXSTRLEN*2];
struct Gadget gadget[NUMGADS];
SHORT gadgetxy[MAXSTRLEN*2][10];

first_gad[NUM_STRIPS] = {0, 12, 17};

char *gadgettxt[NUMGADS] =
{
   "Edit »","Dots","Tunnel","Start","Box","Energy","No-Ghost","Forbid",
         "Create","Del","Draw","Quit",

   "Opts »","Draw Maze","Auto-Draw","Display Sprites","Quit",
   "File »","Load Maze","Load Vars","Save Maze","Save Vars","Quit",
};

#define CREATEGAD       8
#define DELETEGAD       9
#define DOTGAD          1
#define TUNNELGAD       2
#define SAVEGRAPHGAD    20
#define SAVEVARSGAD     21

USHORT gadID[NUMGADS] =
{
   NEXTSTRIP,
   FLAGGADG | DOTS,
   FLAGGADG | TUNNEL,
   FLAGGADG | PACSTART,
   FLAGGADG | GHOSTSTART,
   FLAGGADG | ENERGIZER,
   FLAGGADG | GHOST_1WAY,
   FLAGGADG | FORBID,
   MAKEEDGE,
   DELEDGE,
   DRAWMAZE,
   QUITMAZED,
   NEXTSTRIP,
   DRAWMAZE,
   AUTODRAW,
   DISPLAY_SPRITES,
   QUITMAZED,
   NEXTSTRIP,
   LOADMAZE,
   LOADVARS,
   SAVEMAZE,
   SAVEVARS,
   QUITMAZED,
};
