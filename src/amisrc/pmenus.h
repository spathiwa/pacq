#define PROJ         0
#define PROJ_PLAY    0
#define PROJ_ABOUT   1
#define PROJ_CONST   2
#define PROJ_QUIT    3

#define IWIDTH       96
#define IHEIGHT      8

global struct TextAttr TOPAZ80 = {(STRPTR)"topaz.font",TOPAZ_EIGHTY,0,0};

global struct IntuiText ProjText[] =
{
  {2,1,JAM2,2,1,NULL,"  Play",      NULL},
  {2,1,JAM2,2,1,NULL,"» About",     NULL},
  {2,1,JAM2,2,1,NULL,"  Construct", NULL},
  {2,1,JAM2,2,1,NULL,"  Quit",      NULL},
};

global struct IntuiText ScreenText[] =
{
  {2,1,JAM2,2,1,NULL,"» Screen Mode",NULL},
  {2,1,JAM2,CHECKWIDTH,1,NULL,"Filter Off",NULL},
};

global struct IntuiText ScnModText[] =
{
  {2,1,JAM2,2,1,NULL,"Lo-res",        NULL},
  {2,1,JAM2,2,1,NULL,"Lo-res Laced",  NULL},
  {2,1,JAM2,2,1,NULL,"Hi-res",        NULL},
  {2,1,JAM2,2,1,NULL,"Hi-res Laced",  NULL},
};   

global struct IntuiText AboutText[] =
{
  {2,1,JAM2,2,1,NULL,"PACQ © 1989,92",NULL},
  {2,1,JAM2,2,1,NULL," ",NULL},
  {2,1,JAM2,2,1,NULL,"Aaron Hightower",NULL},
  {2,1,JAM2,2,1,NULL,"1217 Shirley Way",NULL},
  {2,1,JAM2,2,1,NULL,"Bedford, TX 76022",NULL},
  {2,1,JAM2,2,1,NULL," ",NULL},
  {2,1,JAM2,2,1,NULL,"and Ron Romero",NULL},
  {2,1,JAM2,2,1,NULL,"4608 Shady Lake",NULL},
  {2,1,JAM2,2,1,NULL,"Ft Worth, TX 76180",NULL},
};

global struct MenuItem ScnModItem[] =
{
   {&ScnModItem[1], 61,-1, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
     (APTR)&ScnModText[0], NULL, NULL, NULL, MENUNULL
   },
   {&ScnModItem[2], 61,9, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
     (APTR)&ScnModText[1], NULL, NULL, NULL, MENUNULL
   },
   {&ScnModItem[3], 61,19, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
     (APTR)&ScnModText[2], NULL, NULL, NULL, MENUNULL
   },
   {NULL,        61, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
     (APTR)&ScnModText[3], NULL, NULL, NULL, MENUNULL
   }
};

global struct MenuItem AboutItem[] =
{
   {&AboutItem[1], 10,-1, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[0], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[2], 10,9, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[1], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[3], 10,19, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[2], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[4], 10,19, IWIDTH, IHEIGHT, 
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[3], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[5],10, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[4], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[6],10, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[5], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[7],10, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[6], NULL, NULL, NULL, MENUNULL
   },
   {&AboutItem[8],10, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[7], NULL, NULL, NULL, MENUNULL
   },
   {NULL,10, 29, IWIDTH, IHEIGHT,
     ITEMTEXT|ITEMENABLED|HIGHNONE, 0,
     (APTR)&AboutText[8], NULL, NULL, NULL, MENUNULL
   },
};

global struct MenuItem ProjItem[] =
{
   {&ProjItem[1],0,0,IWIDTH,30 + IHEIGHT,
      ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP, 0,
      (APTR)&ProjText[0],NULL,'P',NULL,MENUNULL
   },
   {&ProjItem[2],0,10,IWIDTH,IHEIGHT,
      ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP, 0,
      (APTR)&ProjText[1],NULL,'A',&AboutItem[0],MENUNULL
   },
   {&ProjItem[3],0,20,IWIDTH,IHEIGHT,
      ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP, 0,
      (APTR)&ProjText[2],NULL,'C',NULL,MENUNULL
   },
   {        NULL,0,20,IWIDTH,IHEIGHT,
      ITEMTEXT | COMMSEQ | ITEMENABLED | HIGHCOMP, 0,
      (APTR)&ProjText[3],NULL,'Q',NULL,MENUNULL
   },
};

global struct MenuItem ScreenItem[] =
{
   {&ScreenItem[1],0,30,IWIDTH,IHEIGHT,
      ITEMTEXT | ITEMENABLED | HIGHCOMP, 0,
      (APTR)&ScreenText[0],NULL,NULL,&ScnModItem[0],MENUNULL
   },
   {NULL          ,0,30,IWIDTH,IHEIGHT,
      ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT | CHECKED | MENUTOGGLE, 0,
      (APTR)&ScreenText[1],NULL,'F',NULL,MENUNULL
   },
};

/* Menu Titles */
global struct Menu Menus[] =
{
   {     NULL,0,0,63,0,MENUENABLED,"PACQ",&ProjItem[0]},
/* {&Menus[1],0,0,63,0,MENUENABLED,"PACQ",&ProjItem[0]},
   {     NULL,0,0,63,0,MENUENABLED,"Screen",&ScreenItem[0]}, */
};

global struct Menu *FirstMenu = &Menus[0];
