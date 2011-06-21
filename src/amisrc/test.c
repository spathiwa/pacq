#include <exec/types.h>
#include <exec/exec.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <libraries/dos.h>

struct GfxBase *GfxBase;

main()
{
   if(!(GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",0)))
   {
      printf("No library\n");
      exit(0);
   }
   else
   {

      printf("DxOffset: %d\n",GfxBase->ActiView->DxOffset);
      printf("DyOffset: %d\n",GfxBase->ActiView->DyOffset);
      printf("NormalDisplayRows: %d\n",GfxBase->NormalDisplayRows);
      printf("NormalDisplayColumns: %d\n",GfxBase->NormalDisplayColumns);
      printf("MaxDisplayRow: %d\n",GfxBase->MaxDisplayRow);
      printf("NormalDisplayColumns: %d\n",GfxBase->MaxDisplayColumn);
   }
   
   CloseLibrary((void *)GfxBase);
}
