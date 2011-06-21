#define MYDEBUG         0
#define SCREEN2HEIGHT   12
#define GADSCRN_Y       190

#define BOUND(x,y,z) ( (x) < (y) ) ? (y) :( (x) > (z) ) ? (z) : (x)

#define RoundXY(x,y)  \
{                     \
   x = (x) - offsetx; \
   x = ((x + (SIGN(x) * xfactor>>1)) / xfactor) * xfactor  + offsetx; \
   if(x<0) x=minx;  \
   else if (x>width-1) x=maxx; \
   y = (y) - offsety; \
   y = ((y + (SIGN(y) * yfactor>>1)) / yfactor) * yfactor  + offsety; \
   if(y<0) y=miny;   \
   else if (y>height-1) y=maxy; \
}
#define XORSWAP(a,b) {a ^= b; b ^= a; a ^= b;}

global vertex_pt sv; /* Currently selected vertex */
global char olddir,doflashing,followmouse; /* dir select, & flashcontrol */
global char autodraw,linedrawn,firstflaggad,ADgad,DSgad;
global BOOL in_con;
global ULONG modflags,createflags;
global struct Screen *Screen2;
global struct Window *Window2;
global struct ViewPort *svp2;
global char filename[FCHARS+DSIZE+1];
global short oldrx,oldry;

global void ComputeXYs();
global void SetupGadgets();
global vertex_pt AddConnectVert(short x1,short y1, BOOL vertedge);

extern char   *basicmaze,*basicvars; /* Defined in pdefault.c */
extern char   *title,def_title[]; /* Defined in pm.c */
extern char   sprnums[2][MAXGHOSTS+1];
extern struct SimpleSprite  sprite[MAXGHOSTS+1];
extern struct FileRequester *fileRequester;
extern struct TextFont      *OpenFont(struct TextAttr *);
extern struct ArpBase       *ArpBase;
extern struct TextFont      *defaultFont;
extern struct RastPort      *rastport,*statport;
extern struct View          *curr_View;
extern struct ViewPort      *svp,*viewPort;
extern struct Screen        *PlayScreen,*StatScreen,*Screen;
extern struct Window        *PlayWindow,*StatWindow,*Window;
extern struct NewWindow     NewPlayWindow;
extern struct NewScreen     NewPlayScreen;
extern struct RasInfo       *rasInfo;
extern struct Menu          *FirstMenu;
extern struct TextAttr      topazAttr;

extern vertex_pt v_head;

extern vertex_pt add_vertex(short, short);
extern vertex_pt get_vertex(short, short);

extern char fr_name[FCHARS+1],currentfile[FCHARS+1],fr_path[DSIZE+1];
extern char fr_name[FCHARS+1],fr_path[DSIZE+1];
extern char *title;


