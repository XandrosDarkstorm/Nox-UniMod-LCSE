#include "stdafx.h"
#include "math.h"

extern int (__cdecl *noxWndGetPostion) (void* Window,int *xLeft,int *yTop);
int (__cdecl *noxGuiFontHeightMB) (void* FontPtr);
int (__cdecl *noxSub_43F670) (int Flag); // не уверен
int (__cdecl *noxDrawGetStringSize) (void *FontPtr, const wchar_t*String,int *Width,int *H,int);
int (__cdecl *noxSetRectColorMB) (int);
void *(__cdecl *guiFontPtrByName)(const char *FontName);

void (__cdecl *screenGetSize)(int &X,int &Y);

extern void (__cdecl *drawSetTextColor)(DWORD Color);
extern void (__cdecl *drawString)(void *FontPtr,const wchar_t*String,int X,int Y);
extern void *(__cdecl *gLoadImg)(const char *Name);
extern DWORD parseColor(const char *Color);

const int *noxScreenX=		(int*)0x0069A5E0;
const int *noxScreenY=		(int*)0x0069A5E4;
const int *noxScreenWidth=	(int*)0x0069A5F0;
const int *noxScreenHieght=	(int*)0x0069A5F4;
const int *noxDrawXLeft=	(int*)0x0069A5D0;
const int *noxDrawYTop=		(int*)0x0069A5D4;

DWORD parseColor(const char *Color)
{
	DWORD Ret=0,R=0,G=0,B=0;
	const char *P=Color;
	if (Color==NULL || strcmpi(Color,"TRANSPARENT")==0)
		return 0x80000000;
	if (*P!='#')
		return Ret;
	P++;
	for (int i=1;*P!=0;P++,i++)
	{
		Ret<<=4;
		if (*P>='0' && *P<='9')
			Ret+=(*P)-'0';
		else if (*P>='A' && *P<='F')
			Ret+=(*P)-'A'+10;
		if (i==2)
		{
			R=Ret;Ret=0;
		}else if(i==4)
		{
			G=Ret;Ret=0;
		}else if(i==6)
		{
			B=Ret;Ret=0;
		}
	}
	Ret=(R>>3)<<11 | (G>>2)<<5 |(B>>3);///долбаный 16битный цвет
	Ret|=Ret<<16;
	return Ret;
}
void wstringFromLua(lua_State *L,wchar_t *Dst,int MaxLen)
{
	size_t Len=0;
	const char *Src=lua_tolstring(L,-1,&Len);
	if (Src==NULL)
	{
		wcscpy(Dst,L"(null)");
		return;
	}
	if (Len>MaxLen)
	{
		wcscpy(Dst,L"(long)");
		return;
	}
	mbstowcs(Dst,Src,MaxLen);
}
void *imageFromLua(lua_State *L)
{
	void *Ret=NULL;
	if (lua_type(L,-1)==LUA_TSTRING)
	{
		Ret=gLoadImg(lua_tostring(L,-1));
	}else if (lua_type(L,-1)==LUA_TLIGHTUSERDATA)
	{
		Ret=lua_touserdata(L,-1);
	}
	return Ret;
}
void parseAttr(lua_State *L,int KeyIdx,int ValIdx,void *Struct,const ParseAttrib *Attrs)
{
	byte *H=(byte *)Struct;
	const char *Key=lua_tostring(L,KeyIdx);
	size_t Len;
	const char *Val=lua_tolstring(L,ValIdx,&Len);
	for (ParseAttrib const *P=Attrs;P->name!=NULL;P++)
	{
		if (0!=strcmp(P->name,Key))
			continue;
		switch (P->type)
		{
		case 2: //string
			{
			char *NewString=(char*)noxAlloc(Len+1);
			strcpy(NewString,Val);
			*((char**)( H + P->ofs))=NewString;
			}
			break;
		case 3: //color
			*((DWORD*)( H + P->ofs))=parseColor(Val);
			break;
		case 4:// wstring
			{
				wchar_t *NewString=(wchar_t *)noxAlloc(2*(Len+1));
				mbstowcs(NewString,Val,Len+1);
				*((wchar_t**)( H + P->ofs))=NewString;
			}
			break;
		case 5: //bitfield
			break;//TODO
		}
	}
}

namespace 
{
	int wndScreenSize(lua_State*L)
	{
		int x=0,y=0;
		screenGetSize(x,y);
		lua_pushinteger(L,x);
		lua_pushinteger(L,y);
		return 2;
	}

	int stringGetSizeL(lua_State*L)
	{
		if (lua_type(L,1)!=LUA_TSTRING)
		{
			lua_pushstring(L,"wrong args");
			lua_error_(L);
		}
		const char *V=lua_tostring(L,1);
		wchar_t W[500];
		mbstowcs(W, V,strlen(V));
		W[strlen(V)]=0;
		wchar_t *pW=W;
		int Width=0;
		int Height=0;
		noxDrawGetStringSize(NULL,pW,&Width,&Height,0);
		lua_pushnumber(L,Width);
		lua_pushnumber(L,Height);
		return 2;
	}


	int stringDrawL(lua_State*L)
	{
		if (lua_type(L,1)!=LUA_TSTRING || lua_type(L,2)!=LUA_TNUMBER || lua_type(L,3)!=LUA_TNUMBER)
		{
			lua_pushstring(L,"wrong args");
			lua_error_(L);
		}
		DWORD color=0xFFFFFF;
		if (lua_type(L,4)==LUA_TSTRING)
			color=(int)parseColor(lua_tostring(L,4));
		const char *V=lua_tostring(L,1);
		wchar_t W[500];
		mbstowcs(W, V,strlen(V));
		W[strlen(V)]=0;
		drawSetTextColor(color);
		drawString(NULL,W,lua_tonumber(L,2),lua_tonumber(L,3));
		return 0;
	}

	int screenGetPosL(lua_State*L)
	{
		lua_pushinteger(L,*noxScreenX);
		lua_pushinteger(L,*noxScreenY);
		return 2;
	}

	int screenGetSizeL(lua_State*L)
	{
		lua_pushinteger(L,*noxScreenWidth);
		lua_pushinteger(L,*noxScreenHieght);
		return 2;
	}
	int posWorldToScreenL(lua_State*L)
	{
		if (lua_type(L,1)!=LUA_TNUMBER || lua_type(L,2)!=LUA_TNUMBER)
		{
			lua_pushstring(L,"wrong args!");
			lua_error_(L);
		}
		float x1=lua_tonumber(L,1)-*noxScreenX;
		x1+=*noxDrawXLeft;
		float y1=*noxDrawYTop-*noxScreenY;
		y1+=lua_tonumber(L,2);
		lua_pushinteger(L,x1);
		lua_pushinteger(L,y1);
		return 2;
	}

/*

	int __cdecl DrawConsole(void *WndParam,void *DrawDataParam)
	{
		BYTE *P=(BYTE *) DrawDataParam;
		P+=0x234;P=*((BYTE**)P);
		BYTE *DrawData=P; // esi

		P=(BYTE *) WndParam;
		P+=0x238;P=*((BYTE**)P);
		BYTE *Wnd=P; // edi 

		P=DrawData;
		P+=0x1c;P=*((BYTE**)P);
		int RectColor=(int) P; //eax DrawData->RectColor

		P=DrawData;
		P+=0x14;P=*((BYTE**)P);
		int RectColor2=(int) P; //edx

		P=Wnd;
		P+=0x20;P=*((BYTE**)P);
		BYTE *SomeData=P; //eax Wnd->SomeData

		P=SomeData;
		P+=0x414;P=*((BYTE**)P);
		*P=0; // Wnd->SomeData->+0x414

		wchar_t *Wstr=new wchar_t[255];

		int *xLeft = new int;int *yTop = new int;
		noxWndGetPostion(Wnd,xLeft,yTop); // забиваем иксЛефт и игрекТоп

		P=DrawData;
		P+=0xc8;P=*((BYTE**)P);
		BYTE *FontPtr=P; // eax DrawData->FontPtr

		P=Wnd;
		P+=8;P=*((BYTE**)P);
		int Width= (int) P;

		P=Wnd;
		P+=0xC;P=*((BYTE**)P);
		int Height= (int) P;

		int FontHeight = noxGuiFontHeightMB(FontPtr);
		// в eax потом высота но далее помещает в ecx

		P=Wnd;
		P+=4;P=*((BYTE**)P);
		int Flags=(int) P;

		if (0!=Flags & 0x2000) //Что то проверяет флаг
		noxSub_43F670(1);


		// если не одно то другое и оба третье...
		
		if (0!=Flags & 8) // еще флажок
		{//test    byte ptr [esi], 2
		// jz      short loc_488226  ДОДЕЛАТЬ!!
		}
		else
		{
			P=DrawData;
			P+=0x2C;P=*((BYTE**)P);
			int RectColor2=(int) P;
		}

		/*
		cmp     word ptr [esi+wndStruct.drawData.RectColor3], 0
		lea     eax, [esi+wndStruct.drawData.RectColor3] ; 
		jz      short loc_488287
		


		// else 







		return 1;
	}
*/
}

extern void InjectAddr(DWORD Addr,void *Fn);
extern void InjectOffs(DWORD Addr,void *Fn);
extern void InjectJumpTo(DWORD Addr,void *Fn);
extern void ImageUtilInit();
void guiInit()
{
	ImageUtilInit();
	ASSIGN(screenGetSize,0x00430C50);
	ASSIGN(guiFontPtrByName,0x0043F360);
	ASSIGN(noxGuiFontHeightMB,0x0043F320);
	ASSIGN(noxSub_43F670,0x43F670);
	ASSIGN(noxDrawGetStringSize,0x0043F840);
	ASSIGN(noxSetRectColorMB,0x434460);
	
	registerclient("screenGetPos",screenGetPosL);
	registerclient("screenGetSize",screenGetSizeL);
	registerclient("posWorldToScreen",posWorldToScreenL);
	registerclient("stringDraw",stringDrawL);
	registerclient("stringGetSize",stringGetSizeL);
	registerclient("wndScreenSize",wndScreenSize);

}