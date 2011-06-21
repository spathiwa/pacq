/*** PACVAR2.H - These declarations require some type of #include ********/
/*
/*   By separating these from pacvar.h, simple modules that do not need
/* these vars (from pacvar2.h) can compile faster - Optimally, this
/* file should only be included by the main module, and these vars
/* declared as extern when needed by another module. */
/*************************************************************************/

global struct SimpleSprite sprite[MAXGHOSTS+1];        /* sprite control */
global struct Screen *PlayScreen,*StatScreen;
global struct Window *PlayWindow,*StatWindow;
global struct ViewPort *viewPort;
global struct ViewPort *statViewPort;
global struct View *curr_View;
global struct BitMap bitMap,bitMap2;
global struct BitMap statBitMap;
global struct RasInfo *rasInfo,rasInfo2,*statRasInfo;
global struct RastPort *rastPort,rastPort2,*statRastPort;
global struct ArpBase *ArpBase;
global struct DiskFontBase *DiskFontBase;
global struct TextFont *defaultFont;
global struct TextAttr *loresAttr;
global char fr_name[FCHARS+1],lastgraph[FCHARS+1],fr_path[DSIZE+1];
global struct FileRequester *fileRequester;
global USHORT code;
global struct IntuiMessage *message;
global ULONG  class;
global APTR iaddress;

global BYTE gamebuffer[sizeof(struct InputEvent)];
