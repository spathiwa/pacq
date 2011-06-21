
/** Read8SVX.c **********************************************************
 * 
 * Read a sound sample from an IFF file.  21Jan85 
 * 
 * By Steve Hayes, Electronic Arts. 
 * This software is in the public domain. 
 * 
 * Linkage:
 * LStartup.obj,Read8SVX.o,dunpack.o,iffr.o LIBRARY  LC.lib,Amiga.lib
 *
 ************************************************************************/
 
#include "exec/types.h" 
#include "exec/exec.h" 
#include "libraries/dos.h" 
#include "iff/8svx.h" 
 
/* Message strings for IFFP codes. */ 
char MsgOkay[]        = { "(IFF_OKAY) No FORM 8SVX in the file." }; 
char MsgEndMark[]     = { "(END_MARK) How did you get this message?" }; 
char MsgDone[]        = { "(IFF_DONE) All done."}; 
char MsgDos[]         = { "(DOS_ERROR) The DOS returned an error." }; 
char MsgNot[]         = { "(NOT_IFF) Not an IFF file." }; 
char MsgNoFile[]      = { "(NO_FILE) No such file found." }; 
char MsgClientError[] = { "(CLIENT_ERROR) Read8SVX bug or insufficient RAM."}; 
char MsgForm[]        = { "(BAD_FORM) A malformed FORM 8SVX." }; 
char MsgShort[]       = { "(SHORT_CHUNK) A malformed FORM 8SVX." }; 
char MsgBad[]         = { "(BAD_IFF) A mangled IFF file." }; 
 
/* THESE MUST APPEAR IN RIGHT ORDER!! */ 
char *IFFPMessages[-LAST_ERROR+1] = { 
    /*IFF_OKAY*/  MsgOkay, 
    /*END_MARK*/  MsgEndMark, 
    /*IFF_DONE*/  MsgDone, 
    /*DOS_ERROR*/ MsgDos, 
    /*NOT_IFF*/   MsgNot, 
    /*NO_FILE*/   MsgNoFile, 
    /*CLIENT_ERROR*/ MsgClientError, 
    /*BAD_FORM*/  MsgForm, 
    /*SHORT_CHUNK*/  MsgShort, 
    /*BAD_IFF*/   MsgBad 
    }; 
 
typedef struct { 
   ClientFrame clientFrame; 
   UBYTE foundVHDR; 
   UBYTE pad1; 
   Voice8Header sampHdr; 
   } SVXFrame; 
 
 
/* NOTE: For a simple version of this program, set Fancy to 0. 
 * That'll compile a program that skips all LISTs and PROPs in the input 
 * file. It will look in CATs for FORMs 8SVX. That's suitable for most uses. 
 * 
 * For a fancy version that handles LISTs and PROPs, set Fancy to 1.
 */

#define Fancy  1 
 
BYTE *buf; 
int szBuf; 
 
/** DoSomethingWithSample() **********************************************
 * 
 * Interface to Amiga sound driver. 
 * 
 *************************************************************************/
DoSomethingWithSample(sampHdr)  Voice8Header *sampHdr;  { 
    BYTE *t; 
    printf("\noneShotHiSamples=%ld", sampHdr->oneShotHiSamples); 
    printf("\nrepeatHiSamples=%ld", sampHdr->repeatHiSamples); 
    printf("\nsamplesPerHiCycle=%ld", sampHdr->samplesPerHiCycle); 
    printf("\nsamplesPerSec=%ld", sampHdr->samplesPerSec); 
    printf("\nctOctave=%ld", sampHdr->ctOctave); 
    printf("\nsCompression=%ld", sampHdr->sCompression); 
    printf("\nvolume=0x%lx", sampHdr->volume); 
    /* Decompress, if needed. */ 
    if (sampHdr->sCompression) { 
        t = (BYTE *)AllocMem(szBuf<<1, MEMF_CHIP); 
        DUnpack(buf, szBuf, t); 
        FreeMem(buf, szBuf); 
        buf = t; 
        szBuf <<= 1; 
        }; 
    printf("\nData = %3ld %3ld %3ld %3ld %3ld %3ld %3ld %3ld",  
           buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]); 
    printf("\n       %3ld %3ld %3ld %3ld %3ld %3ld %3ld %3ld ...\n",  
           buf[8+0],buf[8+1],buf[8+2],buf[8+3],buf[8+4],buf[8+5],
           buf[8+6],buf[8+ 7]); 
    } 
 
/** ReadBODY() ***********************************************************
 * 
 * Read a BODY into RAM.  
 * 
 *************************************************************************/
IFFP ReadBODY(context)  GroupContext *context;  { 
    IFFP iffp; 
 
    szBuf  = ChunkMoreBytes(context); 
    buf  = (BYTE *)AllocMem(szBuf, MEMF_CHIP); 
    if (buf == NULL) 
        iffp = CLIENT_ERROR; 
    else 
        iffp = IFFReadBytes(context, (BYTE *)buf, szBuf); 
    CheckIFFP(); 
    } 
 
/** GetFo8SVX() **********************************************************
 * 
 * Called via ReadSample to handle every FORM encountered in an IFF file. 
 * Reads FORMs 8SVX and skips all others. 
 * Inside a FORM 8SVX, it reads BODY. It complains if it 
 * doesn't find an VHDR before the BODY. 
 * 
 * [TBD] We could read and print out any NAME and "(c) " chunks. 
 * 
 *************************************************************************/
IFFP GetFo8SVX(parent)  GroupContext *parent;  { 
   /*compilerBug register*/ IFFP iffp; 
   GroupContext formContext; 
   SVXFrame smusFrame;          /* only used for non-clientFrame fields.*/ 
 
   if (parent->subtype != ID_8SVX) 
      return(IFF_OKAY); /* just continue scaning the file */ 
 
   smusFrame = *(SVXFrame *)parent->clientFrame; 
   iffp = OpenRGroup(parent, &formContext); 
   CheckIFFP(); 
 
   do switch (iffp = GetFChunkHdr(&formContext)) { 
      case ID_VHDR: { 
        smusFrame.foundVHDR = TRUE; 
        iffp = GetVHDR(&formContext, &smusFrame.sampHdr); 
        break; } 
      case ID_BODY: { 
        if (!smusFrame.foundVHDR) 
            iffp = BAD_FORM;            /* Need an VHDR chunk first! */ 
        else iffp = ReadBODY(&formContext); 
        break; } 
      case END_MARK: { 
        if (!smusFrame.foundVHDR) 
            iffp = BAD_FORM; 
        else 
            iffp = IFF_DONE; 
        break; } 
      } while (iffp >= IFF_OKAY);  /* loop if valid ID of ignored chunk or a 
                                * subroutine returned IFF_OKAY (no errors).*/ 
 
    if (iffp != IFF_DONE)  return(iffp); 
 
    /* If we get this far, there were no errors. */ 
    CloseRGroup(&formContext); 
    DoSomethingWithSample(&smusFrame.sampHdr); 
    FreeMem(buf, szBuf); 
    return(iffp); 
    } 
 
/** Notes on extending GetFo8SVX ****************************************
 * 
 * To read more kinds of chunks, just add clauses to the switch statement. 
 * To read more kinds of property chunks (like NAME) add clauses to 
 * the switch statement in GetPr8SVX, too. 
 * 
 ************************************************************************/
 
/** GetPr8SVX() *********************************************************
 * 
 * Called via ReadSample to handle every PROP encountered in an IFF file. 
 * Reads PROPs 8SVX and skips all others. 
 * 
 *************************************************************************/
#if Fancy 
IFFP GetPr8SVX(parent)  GroupContext *parent;  { 
   /*compilerBug register*/ IFFP iffp; 
   GroupContext propContext; 
   SVXFrame *svxFrame = (SVXFrame *)parent->clientFrame; /* Subclass */ 
 
   if (parent->subtype != ID_8SVX) 
      return(IFF_OKAY); /* just continue scaning the file */ 
 
   iffp = OpenRGroup(parent, &propContext); 
   CheckIFFP(); 
 
   do switch (iffp = GetPChunkHdr(&propContext)) { 
      case ID_VHDR: { 
        svxFrame->foundVHDR = TRUE; 
        iffp = GetVHDR(&propContext, &svxFrame->sampHdr); 
        break; } 
      } while (iffp >= IFF_OKAY);  /* loop if valid ID of ignored chunk or a 
                              * subroutine returned IFF_OKAY (no errors).*/ 
 
   CloseRGroup(&propContext); 
   return(iffp == END_MARK ? IFF_OKAY : iffp); 
   } 
#endif 
 
/** GetLi8SVX() **********************************************************
 * 
 * Called via ReadSample to handle every LIST encountered in an IFF file. 
 * 
 *************************************************************************/
#if Fancy 
IFFP GetLi8SVX(parent)  GroupContext *parent;  { 
    SVXFrame newFrame;  /* allocate a new Frame */ 
 
    newFrame = *(SVXFrame *)parent->clientFrame;  /* copy parent frame */ 
 
    return( ReadIList(parent, (ClientFrame *)&newFrame) ); 
    } 
#endif 
 
/** ReadSample() **********************************************************
 * 
 * Read IFF 8SVX, given a file handle open for reading.
 * 
 *************************************************************************/
IFFP ReadSample(file)  LONG file;  { 
   SVXFrame sFrame;     /* Top level "client frame".*/ 
   IFFP iffp = IFF_OKAY; 
 
#if Fancy 
   sFrame.clientFrame.getList = GetLi8SVX; 
   sFrame.clientFrame.getProp = GetPr8SVX; 
#else 
   sFrame.clientFrame.getList = SkipGroup; 
   sFrame.clientFrame.getProp = SkipGroup; 
#endif 
   sFrame.clientFrame.getForm = GetFo8SVX; 
   sFrame.clientFrame.getCat  = ReadICat ; 
 
   /* Initialize the top-level client frame's property settings to the 
    * program-wide defaults. This example just records that we haven't read 
    * any VHDR properties yet. 
    * If you want to read another property, init it's fields in sFrame. */ 
   sFrame.foundVHDR = FALSE; 
   sFrame.pad1      = 0; 
 
   iffp = ReadIFF(file, (ClientFrame *)&sFrame); 
 
   return(iffp); 
   } 
 
/** main0() **************************************************************/
void main0(filename)  char *filename;  { 
    LONG file; 
    IFFP iffp = NO_FILE; 
    file = Open(filename, MODE_OLDFILE); 
    if (file) 
        iffp = ReadSample(file); 
    Close(file); 
    printf(" %s\n", IFFPMessages[-iffp]); 
    } 
 
/** main() ***************************************************************/
void main(argc, argv)  int argc;  char **argv;  { 
    printf("Reading file '%s' ...", argv[1]); 
    if (argc < 2) 
        printf("\nfilename required\n"); 
    else 
        main0(argv[1]); 
    } 

