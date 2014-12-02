/****************************************************************************************
*                                                                                       *
*  4D Serial Sample                                                                     *
*                                                                                       *
*  Date:        13 October 2014                                                         *
*                                                                                       *
*  Description: Demonstrates Pretty much every 4D Serial command.                       *
*               This has been written as a console application.                         *
*                                                                                       *
*               The following files are needed on the uSD to complete all tests. Their  *
*               relative location (from C:\Users\Public\Documents\4D Labs) is shown     *
*               gfx2demo.gci    resources\GC Files                                      *
*               gfx2demo.dat    resources\GC Files                                      *
*               KBFunc.4fn      Picaso ViSi (must be recompiled for Diablo)             *
*               KBFunc.gci      Picaso ViSi                                             *
*               KBFunc.dat      Picaso ViSi                                             *
*               Space.wav       Picaso ViSi Genie\SoundPlayer.ImgData                   *
*                                                                                       *
*  Compile: gcc -o BigDemo BigDemo.c -ldiabloSerial -lm                                 *
*  Run: ./BigDemo                                                                       *
*                                                                                       *
****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fenv.h>
#include <ctype.h>
#include <termios.h>
#include <sys/stat.h>
#include <diabloSerial.h>
#include <Diablo_Types4D.h>
#include <Diablo_const4D.h>
#include <unistd.h>
#include <sys/types.h>
#include "BigDemo.h"
#define min(x, y) ({ \
        typeof(x) _min1 = x; \
        typeof(y) _min2 = y; \
        (void) (&_min1 == &_min2); \
        _min1 <_min2 ? _min1 :  _min2; })

#define SERIALPORT   "/dev/ttyAMA0"
static int comspeed;
#define   gfx2demogci "GFX2DEMO.GCI"
#define   gfx2demodat "GFX2DEMO.DAT"
#define   functest    "KBFUNC.4FN"
#define   functestg   "KBFUNC.gci"
#define   functestd   "KBFUNC.dat"
#define   soundtest   "SPACE.WAV"
#define   testdat     "test.dat"

int fFATtests   ;
int fmediatests ;
int fimgtests   ;
int ftouchtests ;
int floadtests  ;
int fsoundtests ;
int rawbase ;



unsigned char *atoz = {"abcdefghijklmnopqrstuvwxyz"} ;

int errCallback(int ErrCode, unsigned char Errbyte)
{
	printf("Serial 4D Library reports error:  %s", Error4DText[ErrCode]);
	if (ErrCode == Err4D_NAK)
		printf(" returned data = 0x%02X\n", Errbyte) ;
	else
		printf("\n") ;

    //Abort on error?
    if (Error_Abort4D)
        exit(ErrCode);

	return ErrCode;
}

int trymount(void)
{
#define retries 20
	int i ;
	int j ;
	i = file_Mount() ;
	j = 0 ;
	if (!i)
	{
		printf("Please insert the uSD card") ;
		while (   (!i)
		       && (j < retries) )
		{
			printf(".") ;
			i = file_Mount() ;
			j++ ;
		}
	}
	if (j == retries)
		return 0 ;
	else
		return 1 ;
}

int RAWPartitionbase(int * rawbaseo)
{
	unsigned char sector[512] ;
	int rawsize ;
	int FAT ;
	int RAW ;
	int result = 0 ;
	media_SetAdd(0, 0) ;                    // so we test this, could use SetSector instead
	if (!media_RdSector(sector))
		printf("read sector failed\n") ;
	else
	{
		*rawbaseo = 0 ;
		RAW       = 0 ;
		FAT       = 0 ;
		if (   (sector[511] == 0xaa)
		    && (sector[510] == 0x55) )
		{									 // possible partition table
			if (   (   (sector[446] ==   0)
                    || (sector[446] == 0x80) )
			    && (   (sector[462] ==   0)
 			        || (sector[462] == 0x80) )
			    && (   (sector[478] ==   0)
			        || (sector[478] == 0x80) )
			    && (   (sector[494] ==   0)
			        || (sector[494] == 0x80) ) )
			{
				if (   (sector[450] == 0x04)
				    || (sector[450] == 0x06)
				    || (sector[450] == 0x0E) )
				    FAT = 1 ;
				if (sector[466] == 0xDA)
				{
					*rawbaseo = sector[470] + (sector[471] << 8) + (sector[472]  << 16) + (sector[473]  << 24) ;
					rawsize   = sector[474] + (sector[475] << 8) + (sector[476]  << 16) + (sector[477]  << 24) ;
					result    = 1 ;
					RAW       = 1 ;
				}
			}

			else
			{
				if (sector[0] == 0xEB) // jmp instruction
			    	FAT = 1 ;
			    else
					RAW = 1 ; // but no really possible as we couldn't have gotten this far in this program
			}
		}
		else
			RAW = 1 ; // but no really possible as we couldn't have gotten this far in this program
	}
	return result ;
}

void Function_Tests (void)
{
	int i, j ;
	int k ;
	int l ;
	int m ;
	WORD handle, h1, h2 ;
	WORD parms[20];
	unsigned char wks1[255] ;
	unsigned char wks2[255] ;
	unsigned char wks3[255] ;
	gfx_Cls() ;
	putstr("file_Run, file_Exec and\nfile_LoadFunction Tests") ;
	printf("file_Run, file_Exec and file_LoadFunction Tests\n") ;
	j = mem_Heap() ;
	handle = file_LoadFunction(functest) ;
	h1 = writeString(0,  "Please enter your name") ;   // prompts string
	h2 = writeString(h1, "") ;                         // result string, must be max length if following strings are to be 'kept'
	printf("String Handles %d %d\n", h1, h2) ;
	parms[0] = h1 ;                  // prompt string
	parms[1] = h2 ;                  // result string
	parms[2] = 20 ;
	parms[3] = 1 ;                   // 1 = save screen, 0 = don't save screen
	TimeLimit4D = 5000000 ;
	i = file_CallFunction(handle, 4, parms) ;         // calls a function
	printf("You typed %d characters", i) ;
	readString(h2, wks1) ;                              // read string immediately as it will be overwritten 'soon'
	printf(">%s<\n", wks1) ;
	k = mem_Heap() ;
	mem_Free(handle) ;
	l = mem_Heap() ;
	usleep(5000) ;                                       // give time to read the 'restored' screen

	h1 = writeString(0,  "Please type anything") ;     // prompts string
	h2 = writeString(h1, "") ;                         // result string, must be max length if following strings are to be 'kept'
	parms[0] = h1 ;                  // prompt string
	parms[1] = h2 ;                  // result string
	parms[2] = 20 ;
	parms[3] = 0 ;                   // 1 = save screen, 0 = don't save screen
	i = file_Exec(functest, 4, parms) ;
	readString(h2, wks2) ;                              // read string immediately as it will be overwritten 'soon'
	gfx_Cls() ;

	h1 = writeString(0,  "Please some more") ;         // prompts string
	h2 = writeString(h1, "") ;                         // result string, must be max length if following strings are to be 'kept'
	parms[0] = h1 ;                  // prompt string
	parms[1] = h2 ;                  // result string
	parms[2] = 20 ;
	parms[3] = 0 ;                   // 1 = save screen, 0 = don't save screen
	i = file_Run(functest, 4, parms) ;
	readString(h2, wks3) ;                              // read string immediately as it will be overwritten 'soon'
	gfx_Cls() ;
	m = mem_Heap() ;
	printf("Memfree before loadfunction = %d\n", j) ;
	printf("Memfree after loadfunction = %d\n", k) ;
	printf("Memfree after free = %d\n", l) ;
	printf("Memfree at end = %d\n", m) ;
	printf("You typed\n") ;
	printf(wks1) ;
	printf("\n") ;
	printf(wks2) ;
	printf("\n") ;
	printf(wks3) ;
	printf("\n") ;
	TimeLimit4D = 2000 ;
}

void gfx_Part1(void)
{
	int i ;
	gfx_BGcolour(LIGHTGOLD) ;           // to check CLS works with different bg color
	gfx_Cls() ;
	txt_BGcolour(LIGHTGOLD) ;           // to ensure text goesn look odd
	txt_FGcolour(RED) ;
	putstr("gfx_A to gfx_L") ;
	printf("gfx_A to gfx_L\n") ;
	txt_FGcolour(LIME) ;            // reset
	gfx_BevelShadow(1) ;                // make it really dark
	gfx_BevelWidth(6) ;                 // make the button bigger by increasing the bevel size
	for (i = 1; i <= 10; i++)
	{
		gfx_Button(ON, 120,50, YELLOW, PURPLE, FONT3, 1, 1, "Test Button") ;
		usleep(100) ;
		gfx_Button(OFF, 120,50, YELLOW, PURPLE, FONT3, 1, 1, "Test Button") ;
		usleep(100) ;
	}
	gfx_BevelShadow(3) ; // back to default
	gfx_ChangeColour(LIME, WHITE) ;
	gfx_Circle(30,30,10,BLUE) ;
	gfx_CircleFilled(130,30,10,BLUE) ;
	gfx_Rectangle(60,60,100,100,RED) ;  // draw a rectange to show where we are clipping
	gfx_ClipWindow(60,60,100,100) ;
	gfx_Clipping(ON) ;                  // turn clipping on but just use it for text
	gfx_MoveTo(40,80) ;
	putstr("1234567890asdfghjkl") ;     // this is clipped
	gfx_Clipping(OFF) ;
	usleep(1000) ;
	printf("Display off\n") ;
	gfx_Contrast(0) ;
	usleep(1000) ;
	gfx_Contrast(15) ;
	printf("Display on\n") ;
	gfx_Ellipse(100,230, 50,30,RED) ;
	gfx_EllipseFilled(100,300, 50,30,AQUA) ;
	gfx_FrameDelay(6) ;
	printf("X Res= %d  Y Res= %d\n",gfx_Get(X_MAX)+1, gfx_Get(Y_MAX)+1) ;
	printf("Pixel at 0,30 is %4.4x\n", gfx_GetPixel(0, 30)) ;
	gfx_Line(0,0,100,200,BLUE) ;
	gfx_LinePattern(0x00aa) ;
	gfx_Set(OBJECT_COLOUR, WHITE);
	gfx_LineTo(239,319) ;
	gfx_LinePattern(0) ;            // reser
	gfx_BGcolour(BLACK) ;           // reset
	txt_BGcolour(BLACK) ;           // reset
}

void gfx_Part2(void)
{
	int i ;
	int k ;
	int l ;
	WORD orbitx ;
	WORD orbity ;
	WORD vx[20] ;    // : array[0..19] of word ;
	WORD vy[20] ; //     : array[0..19] of word ;
	gfx_Cls() ;
	putstr("gfx_M to gfx_T") ;
	printf("gfx_M to gfx_T") ;
	k = 180 ;
	l = 80 ;
	gfx_MoveTo(k, l);
	gfx_CircleFilled(k,l,5,BLUE) ;
	i = -90;   // 12 o'clock position
	while (i<270)
	{
		gfx_Orbit(i, 30, &orbitx, &orbity);
		k = 3;
		if ((i % 90) == 0 )
			k = 5;
		gfx_Circle(orbitx , orbity, k, BLUE);
		i += 30;   // each 30 degreees
	}

	gfx_OutlineColour(YELLOW) ;
	gfx_Panel(PANEL_RAISED,140,0,190,20, LIME) ;
	gfx_OutlineColour(0) ;                    // turn outline off

	vx[0] = 36;   vy[0] = 110;
	vx[1] = 36;   vy[1] = 80;
	vx[2] = 50;   vy[2] = 80;
	vx[3] = 50;   vy[3] = 110;
	vx[4] = 76;   vy[4] = 104;
	vx[5] = 85;   vy[5] = 80;
	vx[6] = 94;   vy[6] = 104;
	vx[7] = 76;   vy[7] = 70;
	vx[8] = 85;   vy[8] = 76;
	vx[9] = 94;   vy[9] = 70;
	vx[10] = 110; vy[10] = 66;
	vx[11] = 110; vy[11] = 80;
	vx[12] = 100; vy[12] = 90;
	vx[13] = 120; vy[13] = 90;
	vx[14] = 110; vy[14] = 80;
	vx[15] = 101; vy[15] = 70;
	vx[16] = 110; vy[16] = 76;
	vx[17] = 119; vy[17] = 70;
	// house
	gfx_Rectangle(6,50,66,110,RED);             // frame
	gfx_Triangle(6,50,36,9,66,50,YELLOW);       // roof
	gfx_Polyline(4, vx, vy, CYAN);            // door
	// man
	gfx_Circle(85, 56, 10, BLUE);               // head
	gfx_Line(85, 66, 85, 80, BLUE);             // body
	gfx_Polyline(3, &vx[4], &vy[4], CYAN);      // legs
	gfx_Polyline(3, &vx[7], &vy[7], BLUE);      // arms
	// woman
	gfx_Circle(110, 56, 10, PINK);              // head
	gfx_Polyline(5, &vx[10], &vy[10], BROWN);   // dress
	gfx_Line(104, 104, 106, 90, PINK);          // left arm
	gfx_Line(112, 90, 116, 104, PINK);          // right arm
	gfx_Polyline(3, &vx[15], &vy[15], SALMON);  // dress

	vx[0] = 10; vy[0] = 130;
	vx[1] = 35; vy[1] = 125;
	vx[2] = 80; vy[2] = 130;
	vx[3] = 60; vy[3] = 145;
	vx[4] = 80; vy[4] = 160;
	vx[5] = 35; vy[5] = 170;
	vx[6] = 10; vy[6] = 160;
	gfx_Polygon(7, vx, vy, RED);

	vx[0] = 110; vy[0] = 130;
	vx[1] = 135; vy[1] = 125;
	vx[2] = 180; vy[2] = 130;
	vx[3] = 160; vy[3] = 145;
	vx[4] = 180; vy[4] = 160;
	vx[5] = 135; vy[5] = 170;
	vx[6] = 110; vy[6] = 160;
	gfx_PolygonFilled(7, vx, vy, RED);

	gfx_PutPixel(40, 94, LIME) ;          // door knob
	gfx_Rectangle(0,180, 10,200, AQUA) ;
	gfx_RectangleFilled(20,180, 40,200, ORANGE) ;
	gfx_ScreenCopyPaste(0,0, 0,280, 40,40) ;
	gfx_ScreenMode(LANDSCAPE) ;
	//gfx_Set(CLIPPING, ON) ;
	//gfx_SetClipRegion() ;
	gfx_Slider(SLIDER_RAISED, 210, 100, 250,10, BLUE, 100, 50) ; // coordinates are different because we are in landscape mode
	gfx_ScreenMode(PORTRAIT) ;
	gfx_Transparency(ON) ;
	gfx_TransparentColour(YELLOW) ;  // how do we 'test' this?
	gfx_Triangle(6,250, 36,209, 66,250,YELLOW);
	gfx_TriangleFilled(110,210, 130,210, 120,230,CYAN);
}

void text_Tests(void)
{
	gfx_Cls() ;
	printf("Text Tests\n") ;
	putstr("Text Tests") ;

	txt_Attributes(BOLD + INVERSE + ITALIC + UNDERLINED) ;
	txt_Xgap(3) ;
	txt_Ygap(3) ;
	txt_BGcolour(YELLOW) ;
	txt_FGcolour(WHITE) ;
	txt_FontID(FONT3) ;
	txt_MoveCursor(5, 0) ;
	putstr("Hello There") ;

	txt_MoveCursor(6, 2) ;
	txt_Height(2) ;
	txt_Width(2) ;
	txt_Inverse(OFF) ;
	txt_Italic(OFF) ;
	txt_Opacity(TRANSPARENT) ;
	txt_Set(TEXT_COLOUR, LIME) ;
	txt_Underline(ON) ;
	txt_Bold(OFF) ;
	txt_Wrap(88) ;
	putstr("Hello There") ;
	txt_Height(1) ;
	txt_Width(1) ;
	putCH('z') ;
	txt_Wrap(0) ;              // reset
	printf("Char height= %d Width= %d\n", charheight('w'), charwidth('w') ) ;
	txt_BGcolour(BLACK) ;
	txt_FGcolour(LIME) ;
	txt_FontID(FONT3) ;
	txt_MoveCursor(0,0) ;      // reset
}

void FAT_Tests(void)
{
	int i ;
	int j ;
	int k , handle;
	WORD w1, w2 ;
	unsigned char wks[255] ;
	unsigned char bytes[20] ;
	struct datar data ;
	gfx_Cls() ;
	printf("FAT Tests\n") ;
	putstr("FAT Tests\n") ;
	printf("File Error= %d\n", file_Error()) ;
	printf("uSD has %d Files\n", file_Count("*.*")) ;
	file_Dir("*.dat") ;     // should this get returned!? FindFirst and next certainly should, both need to be manual as they need "to(buffer)"

	if (file_Exists(testdat))
		file_Erase(testdat) ;
	handle = file_Open(testdat, 'w') ;
	printf("Handle= %d\n",handle) ;
	// write some stuff to uSD
	file_PutC('a', handle) ;
	file_PutW(1234, handle) ;
	file_PutS("This is a Test", handle) ;
	file_Close(handle) ;

	handle = file_Open(testdat, 'r') ;
	printf("Handle= %d\n",handle) ;
	// read it back and dump to screen
	printf("%c\n",file_GetC(handle)) ;
	printf("%d\n",file_GetW(handle)) ;
	i = file_GetS(wks, 100, handle) ;
	printf("Length=%d, String=""%s""\n", i, wks) ;

	file_Rewind(handle) ;
	i = file_Read(bytes, 10, handle) ;
	printf("Bytes read= %d Data=", i) ;
	for (j = 0; j <= i-1; j++)
		printf("%2.2x ", bytes[j]) ;
	i = file_Tell(handle, &w1, &w2) ;
	printf("\nFile pointer= %d\n", (w1 << 16) + w2) ;
	i = file_Size(handle, &w1, &w2) ;
	printf("File size=%d\n", (w1 << 16) + w2) ;

	file_Close(handle) ;
	file_Erase(testdat) ;

	handle = file_Open(testdat, 'w') ;
	printf("Handle=%d\n",handle) ;
	for(i = 1; i <= 50; i++)
	{
		data.recnum = i ;
		k = i % 20 ;
		for (j = 0; j <= 4; j++)
		{
			data.values[j] = atoz[k+j] ;
			data.idx = atoz[rand() % 27] ;
		}
		file_Write(sizeof(data), data.values, handle) ;
	}
	file_Close(handle) ;
	handle = file_Open(testdat, 'r') ;
	file_Index(handle, sizeof(data) >> 16, sizeof(data) & 0xFFFF, 5) ;
	i = file_Read(data.values, sizeof(data), handle) ;
	printf("%d %c %c %c %c %c %c\n", data.recnum, data.values[0],data.values[1],data.values[2],data.values[3],data.values[4], data.idx) ;
	file_Seek(handle, 0, 10*sizeof(data)) ;
	i = file_Read(data.values, sizeof(data), handle) ;
	printf("%d %c %c %c %c %c %c\n", data.recnum, data.values[0],data.values[1],data.values[2],data.values[3],data.values[4], data.idx) ;
	file_Close(handle) ;
	file_Erase(testdat) ;


	file_FindFirstRet("*.dat", wks) ;
	printf(wks) ;
	printf("\n") ;
	file_FindNextRet(wks) ;
	printf(wks) ;
	printf("\n") ;

	handle = file_Open(testdat, 'w') ;
	printf("Handle=%d\n",handle) ;
	i = sizeof(Image) ;
	k = 0 ;
	while (i != 0)
	{
		j = min(512, i);
		file_Write(j, &Image[k], handle) ;
		i -= j ;
		k += j ;
	}
	file_Close(handle) ;
	gfx_Cls() ;
	handle = file_Open(testdat, 'r') ;
	file_Image(0,0,handle) ;
	file_Close(handle) ;
	gfx_MoveTo(40,10) ;
	putstr("4D Logo") ;

	file_Erase(testdat) ;
	handle = file_Open(testdat, 'w') ;
	printf("Handle=%d", handle) ;
	file_ScreenCapture(0,0,100,32, handle) ;
	file_Close(handle) ;

	handle = file_Open(testdat, 'r') ;
	file_Image(0,40,handle) ;
	file_Rewind(handle) ;
	file_Image(0,80,handle) ;
	file_Rewind(handle) ;
	file_Image(0,120,handle) ;
	file_Close(handle) ;
	file_Erase(testdat) ;
}

void IMG_Tests(void)
{
	WORD handle ;
	int i ;
	int j ;
	int k ;
	gfx_Cls() ;
	txt_MoveCursor(0, 5) ;
	putstr("IMG Tests") ;
	printf("IMG Tests") ;
	handle = file_LoadImageControl("GFX2DEMO.DAT", "GFX2DEMO.GCI", 1) ;
	printf("%d\n", handle) ;
	for (i = 0; i <= 4; i++)  // 4 is "default", same as no dark/light ening
	{
		gfx_BevelShadow(i) ;
		img_Darken(handle, 0) ; // bug, darkens atm
		img_Show(handle, 0) ;
		usleep(250) ;
	}
	for (i = 3; i >= 0; i--)
	{
		gfx_BevelShadow(i) ;
		img_Lighten(handle, 0) ;
		img_Show(handle, 0) ;
		usleep(250) ;
	}
	gfx_BevelShadow(3) ; // back to default

	img_SetPosition(handle, 0, 0, 50) ; // move to a different position
	img_Show(handle, 0) ;

	j = img_GetWord(handle, 0, IMAGE_FRAMES) ;
	for (i = 0; i < j; i++)
	{
		img_SetWord(handle, 0, IMAGE_INDEX, i) ;
		img_Show(handle, 0) ;
		usleep(500) ;
	}

	img_Disable(handle, ALL) ;
	j = 0 ;
	k = 0 ;
	for (i = 36; i <= 39; i++)
	{
		img_SetPosition(handle, i, j, k) ; // move to a different position
		if (j == 119)
		{
			k = 149 ;
			j = 0 ;
		}
		else
			j = 119 ;
		img_Enable(handle, i) ;
	}
	img_Show(handle,ALL) ;
	//  img_ClearAttributes(handle, index, value) ;
	//  img_SetAttributes(handle, index, value) ;
	if (ftouchtests)
	{
		touch_Set(TOUCH_ENABLE) ;
		printf("Please Touch an Image\n") ;
		i = -1 ;
		do
		{
			j = touch_Get(TOUCH_STATUS) ;
			if (j == TOUCH_PRESSED)
				i = img_Touched(handle, ALL) ;
		} while (i == -1) ;
		printf("You touched Image Index %d\n", i) ;
	}
	mem_Free(handle) ;
}

void Media_Tests(void)
{
	int i ;
	int j ;
	int k ;
	int l ;
	int m , handle ;
	unsigned char sector[512] ;
	gfx_Cls() ;
	putstr("Media Tests") ;
	printf("Media Tests\n") ;
	file_Unmount() ;    // just to test this and media_Init
	i = media_Init() ;
	if (i == 0)
	{
		printf("Please insert the uSD card") ;
		while (i = 0)
		{
			printf(".") ;
			i = media_Init() ;
		}
	}

	printf("First RAW sector=%d\n", rawbase) ;
	trymount() ;

	handle = file_Open("GFX2DEMO.GCI", 'r') ;
	file_Seek(handle, 0x49, 0x5800) ;   // location of large unicorn file
	i = 128 * 128 * 13 * 2 + 8 ;     // size of large unicorn file
	l = (i / 512) + 1 ;
	// we assume here that the raw partition is big enough to write this, could
	k = rawbase ;
	m = 1 ;
	while (i != 0)
	{
		printf("Copying sector %d of %d\r", m, l) ;
		j = min(512, i) ;
		file_Read(sector, j, handle) ;
		media_SetSector(k >> 16, k & 0xFFFF) ;
		k++ ;
		media_WrSector(sector) ;
		i -= j ;
		m++ ;
	}
	file_Close(handle) ;
	media_SetSector(rawbase >> 16, rawbase & 0xFFFF) ;
	media_Image(0,0) ;
	media_SetSector(rawbase >> 16, rawbase & 0xFFFF) ;
	media_Video(0,128) ;

	media_SetSector(rawbase >> 16, rawbase & 0xFFFF) ;
	media_WriteByte(0x11) ;
	media_WriteWord(0x2233) ;
	media_Flush() ;            // should write 0xFF over the rest of the sector
	media_SetSector(rawbase >> 16, rawbase & 0xFFFF) ;
	printf("\n%2.2x %4.4x %4.4x\n",media_ReadByte(), media_ReadWord(), media_ReadWord());
}

void Sound_Tests(void)
{
	int i ;
	int j ;
	gfx_Cls() ;
	printf("Sound Tests\n") ;
	putstr("Sound Tests") ;
	snd_Volume(127) ;
	snd_BufSize(2) ;
	printf("Playing\n") ;
	file_PlayWAV(soundtest) ;
	usleep(10000) ;
	printf("Pausing\n") ;
	snd_Pause() ;
	usleep(5000) ;
	printf("Continuing\n") ;
	snd_Continue() ;
	usleep(5000) ;
	printf("Playing with pitch\n") ;
	i = snd_Pitch(0xFFFF) ;
	printf("Original Pitch=%d\n",i) ;
	usleep(5000) ;
	snd_Pitch(trunc(i*2)) ;
	usleep(5000) ;
	snd_Pitch(trunc(i/2)) ;

	usleep(5000) ;
	snd_Pitch(i) ;
	usleep(5000) ;
	for (j = 1; j <= 5; j++)
	{
		i = snd_Playing() ;
		printf("Blocks remaining=%d\r ",i) ;
		usleep(50000) ;
	}
	printf("\n") ;
	for (i = 127; i >= 8; i--)
	{
		snd_Volume(i) ; // 8 to 127 ;
		printf("Volume=%d\r ",i) ;
		usleep(50000) ;
	}
	printf("\nStopping\n") ;
	snd_Stop() ;
}

void Touch_Tests(void)
{
	int firstx ;
	int firsty ;
	int x ;
	int y ;
	int state ;
	gfx_Cls() ;
	putstr("Touch Tests\n") ;
	printf("Touch Tests.\n") ;
	putstr("Please ensure Touch is only\ndetected in the Blue area") ;
	printf("Detecting touch in Region\n") ;
	touch_Set(TOUCH_ENABLE) ;
	touch_DetectRegion(100,100, 200, 200) ;
	gfx_RectangleFilled(100,100, 200, 200, BLUE) ;
	do {} while (touch_Get(TOUCH_STATUS) != TOUCH_PRESSED);
	touch_Set(TOUCH_REGIONDEFAULT) ;
	gfx_Cls() ;
	putstr("Draw.. Drawing stops\nwhen touch released\n") ;
	printf("Drawing\n") ;

	while(touch_Get(TOUCH_STATUS) != TOUCH_PRESSED)
	{      // we"ll wait for a touch
	}
	firstx = touch_Get(TOUCH_GETX);                          // so we can get the first point
	firsty = touch_Get(TOUCH_GETY);
	while(state != TOUCH_RELEASED)
	{
		state = touch_Get(TOUCH_STATUS);                       // look for any touch activity
		x = touch_Get(TOUCH_GETX);                             // grab the x
		y = touch_Get(TOUCH_GETY);                             // and the y coordinates of the touch
		if (state == TOUCH_PRESSED)                               // if there"s a press
		{
			firstx = x;
			firsty = y;
		}

		if (state == TOUCH_MOVING)                                // if there"s movement
		{
			gfx_Line(firstx, firsty, x, y, BLUE);                 // but lines are much better
			firstx = x;
			firsty = y;
		}
	}
	putstr("Done!\n") ;
	touch_Set(TOUCH_DISABLE) ;
}


int main() 
{
    int i, rc;
    char wks[20] ;
    TimeLimit4D = 2000;
    Callback4D = errCallback;
    // abort on detected 4D Serial error
    Error_Abort4D = 1 ;
    comspeed = 9600;
    rc = OpenComm(SERIALPORT, comspeed);
    if (rc != 0)
    {
        printf("Error %d Opening: %s - %s\n", errno, SERIALPORT, strerror(errno));
        exit(EXIT_FAILURE);
    }

	gfx_Cls() ;
	fFATtests   = trymount() ;
	fmediatests = 0 ;
	fimgtests   = 0 ;
	ftouchtests = 0 ;
	floadtests  = 0 ;
	fsoundtests = 0 ;
	sys_GetModel(wks) ; // length is also returned, but we don't need that here
	printf("Display model: %s\n", wks) ;
	putstr("Display model: ") ;
    putstr(wks) ;
	i = strlen(wks) ;
	if (   (wks[i] == 'T')
	    || (wks[i-1] == 'T') )
		ftouchtests = 1 ;
	printf("SPE2 Version: %4.4x\n", sys_GetVersion()) ;
	printf("PmmC Version: %4.4x\n", sys_GetPmmC()) ;
	if (fFATtests)
	{
		if (RAWPartitionbase(&rawbase))
			fmediatests = 1 ;
		if (   (file_Exists(gfx2demodat) != 0)
		    && (file_Exists(gfx2demogci) != 0) )
			fimgtests = 1 ;
		if (   (file_Exists(functest) != 0)
		    && (file_Exists(functestg) != 0)
		    && (file_Exists(functestd) != 0) )
			floadtests = 1 ;
		if (file_Exists(soundtest))
			fsoundtests = 1 ;
	}
	if (fFATtests)
	{
		printf("FAT Tests will be done\n") ;
		if (fmediatests)
		{
			if (file_Exists(gfx2demogci))
		  		printf("Media tests will be done\n") ;
		  	else
			{
		    printf("Media tests cannot be done, missing %s\n", gfx2demogci) ;
		    fmediatests = 0 ;
		    }
		}
		else
		   printf("Media tests cannot be done, no RAW Partition\n") ;
		if (fimgtests)
			printf("Image tests will be done\n") ;
		else
			printf("Image tests will not be done, missing %s or %s\n", gfx2demogci, gfx2demodat) ;
		if (floadtests)
			printf("Load tests will be done\n") ;
		else
			printf("Load tests will not be done, missing %s or %s or %s",functest, functestg, functestd) ;
		if (fsoundtests)
			printf("Sound tests will be done\n") ;
		else
		    printf("Sound tests will not be done, missing %s\n", soundtest) ;
	}
	else
	{
		printf("FAT Tests cannot be done, either no uSD card or no FAT partition\n") ;
		printf("Neither will Media, Image, Load or Sound Tests\n") ;
	}
	if (ftouchtests)
		printf("Touch Tests will be done\n") ;
	else
		printf("Touch Tests will not be done, display doesn't appear capable\n") ;

	usleep(5000) ;

	gfx_Part1() ; // GFX Part 1 tests
	usleep(5000) ;

	gfx_Part2() ; // GFX Part 2 tests
	usleep(5000) ;

	text_Tests() ; // text tests
	usleep(5000) ;

	if (fFATtests)
	{
		FAT_Tests() ;
		usleep(5000) ;
	}

	if (fimgtests)
	{
		IMG_Tests() ;
		usleep(5000) ;
	}

	if (fmediatests)
	{
		Media_Tests() ;
		usleep(5000) ;
	}

	if (floadtests)
	{
		Function_Tests() ;
		usleep(5000) ;
	}

	if (ftouchtests)
	{
		Touch_Tests() ;
		usleep(5000) ;
	}

	if (fsoundtests)
	{
		Sound_Tests() ;
		usleep(100000) ;
	}
                
    return (0);
}

