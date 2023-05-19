#include "stdafx.h"
#include "unit.h"
#include "noxManifest.h"

extern void (__cdecl *netClientSend) (int PlrN,int Dir,//1 - клиенту
								void *Buf,int BufSize);

extern void (__cdecl *wndShowHide)(void *Wnd,int Hide);

extern DWORD *frameCounter;


void (__cdecl *sub_4C31D0) (int netCode);
void (__cdecl *sub_4C3147)();
void (__cdecl *sub_4C2BF0)();
void (__cdecl *cliSummondWndLoad) ();
void (__cdecl *wndSummonCreateList) (void*);

BYTE *wndSummonUsed;
int wndConjSummonMsg;
int *creatureSummonCommandAll;



struct creatureWhatDo 
{
	int commandAll;
	int mobNetCode[4];
	int command[4];
};

creatureWhatDo myCreatureList;


namespace
{
	/// <summary>
	/// Deletes DeathBall object, if it gets stuck inside Great Sword for more than 4 frames.
	/// </summary>
	void __declspec(naked) asmDeathBallBugGs()
	{
		/*
		* X.D.: I have translated original commentaries, but i don't have firm idea what trickery
		* is going on.
		* I believe what is happening is that the code checks for collision framestamp
		*/
		__asm
		{
			call noxUnitSetOwner
			add esp,0Ch
			cmp [ebx+4],02C2h
			jnz exitL
			mov eax,[ebx+2DCh] // last collision time
			mov ecx,frameCounter
			mov ecx,[ecx+0]
			test eax,eax // if collision time is zero, keep going
			jnz l1
			obnul:
			mov [ebx+2DCh],ecx
			mov [ebx+2E0h],0
			jmp exitL
			l1: //if collision time is not zero - check frames
			sub ecx,eax
			cmp ecx,4
			ja obnul // if more than 4 frames passed jump to obnul
			mov ecx,[ebx+2E0h]
			inc ecx
			mov [ebx+2E0h],ecx
			cmp ecx,3
			jna exitL
			push ebx
			call noxDeleteObject
			add esp,4

			exitL:
			push 004E1C92h
			ret
		}
	}

	/// <summary>
	/// Deletes DeathBall object, if it gets stuck inside Warrior's shield for more than 4 frames.
	/// </summary>
	void __declspec(naked) asmDeathBallBugSh()
	{
		//This code is the same as asmDeathBallBugGs, except for one value, commented in the code.
		__asm
		{
			call noxUnitSetOwner
			add esp,0Ch
			cmp [ebx+4],02C2h
			jnz exitL
			mov eax,[ebx+2DCh]
			mov ecx,frameCounter
			mov ecx,[ecx+0]
			test eax,eax
			jnz l1
			obnul:
			mov [ebx+2DCh],ecx
			mov [ebx+2E0h],0
			jmp exitL
			l1:
			sub ecx,eax
			cmp ecx,4
			ja obnul
			mov ecx,[ebx+2E0h]
			inc ecx
			mov [ebx+2E0h],ecx
			cmp ecx,3
			jna exitL
			push ebx
			call noxDeleteObject
			add esp,4

			exitL:
			push 004E1BDEh //This value is different
			ret
		}
	}


	/// <summary>
	/// Allows right click on creature inside Summoned Creatures window
	/// </summary>
	void __declspec(naked) asmConjSummonAnotherCmp()
	{
		__asm
		{
			cmp eax,9
			mov wndConjSummonMsg,eax
			jnz l1
			push 004C2B32h
			ret
			
			l1:
			xor eax,eax
			add esp,10h
			retn
		}
	}
	struct creatureCommand
	{
		BYTE msg;
		short netCodeMob;
		short whatDo;
	};

	void __cdecl conjSummonDo(int A,void *B)
	{
		BYTE *M=*((BYTE**)wndSummonUsed);
		int Top=lua_gettop(L);
		getServerVar("creatureSummonWhatDo");
		if (lua_type(L,-1)==LUA_TNIL)
		{
			wndSummonCreateList(B);
			lua_settop(L,Top);
			return;
		}
		int ForHunt=0;
		if (lua_tointeger(L,-1)==1)
			ForHunt=1;
		creatureCommand P;
		P.msg=0x78;
		P.netCodeMob=(short)*((BYTE**)M);
		int command=0;
		if (A==1)
			P.whatDo=0; // испариться
		else
		{
			for (int i=0;i<4;i++)
			{
				if (P.netCodeMob==myCreatureList.mobNetCode[i])
				{
					command=myCreatureList.command[i];
					if (command<3)
						P.whatDo=4;
					else if(command>=4+ForHunt)
						P.whatDo=3;
					else
						P.whatDo=myCreatureList.command[i]+1;
					myCreatureList.command[i]=P.whatDo;
				}
			}
		}
		netClientSend(0x1F,0,&P,4);
		lua_settop(L,Top);
	}
	
	/// <summary>
	/// This code determines what to do, when player clicks on creature in Summoned Creatures window.
	/// </summary>
	void __declspec(naked) asmConjSummonDo()
	{
		__asm
		{
			add esp,4
			push eax
			mov eax,wndConjSummonMsg
			cmp eax,9
			jz l1 //if we right clicked on it - order BANISH.
			push 0 //if we are here - we made a left click and cycle through orders
			call conjSummonDo
			jmp lex
			l1:
			push 1
			call conjSummonDo

			lex: // returning from subroutine
			add esp,8
			push 004C2BBEh
			ret
		}
	}

	void __cdecl conjSummonCreate(int netCode)
	{
		for (int i=0;i<4;i++)
		{
			if (myCreatureList.mobNetCode[i]==0)
			{
				myCreatureList.mobNetCode[i]=netCode;
				myCreatureList.command[i]=myCreatureList.commandAll;
				return;
			}
		}
	}
	void __declspec(naked) asmConjSummonCreate() // когда создали моба прогоняем его
	{
		__asm
		{
			push esi 
			call conjSummonCreate
			add esp,4
			call cliSummondWndLoad
			push 0049179Fh
			ret
		}
	}

	void __cdecl ConjSummonDoAll()
	{
		int c=myCreatureList.commandAll;
		for (int i=0;i<4;i++)
			myCreatureList.command[i]=c;
		if (c==0) 
			myCreatureList.commandAll=4;
	}
	void __declspec(naked) asmConjSummonDoAll() // когда отпровляем команду всем
	{
		__asm
		{
			xor eax,eax
			mov al,cl
			mov [myCreatureList+0],eax
			call ConjSummonDoAll
			call netClientSend
			push 004C2AD1h
			ret
		}
	}

	void __cdecl ConjSummonLoadWnd()
	{
		myCreatureList.commandAll=4;
		for (int i=0;i<4;i++)
		{
			myCreatureList.command[i]=0;
			myCreatureList.mobNetCode[i]=0;
		}
	}
	void __declspec(naked) asmConjSummonLoadWnd() // когда заргружается окна все к черту обнуляем
	{
		__asm
		{
			call wndShowHide
			call ConjSummonLoadWnd
			push 004C1FA6h
			ret
		}
	}

	void __cdecl ConjSummonDieOrBanish(int netCode)
	{
		for  (int i=0;i<4;i++)
		{
			if (myCreatureList.mobNetCode[i]==netCode)
				myCreatureList.mobNetCode[i]=0;
		}
	}
	void __declspec(naked) asmConjSummonDieOrBanish() // когда моб сдыхает
	{
		__asm
		{
			push edi
			call ConjSummonDieOrBanish
			add esp,4
			call sub_4C31D0
			push 004C314Ch
			ret
		}
	}
	void __cdecl fixCastFieball(float *A,float *B)
	{
		float *X=*((float**)A);
		int *XMouse=*((int**)B);
		float *Y=X+1;
		int *YMouse=XMouse+1;
		int cosA=*XMouse-*X;
		int sinA=*YMouse-*Y;
		float dist=sqrt((float)(cosA^2+sinA^2));
		*A=dist/cosA;
		*B=dist/sinA;
	}

	void __declspec(naked) asmFixCastFireball() // 0052C7CD
	{
		__asm
		{
			// edi - unit
			test byte ptr [edi+8],4 // игрок ли? Мало-ли что, мб у меня параноя
			jz l1
		/*	mov edx,edi
			add edx,0x38
			push edx
			mov edx,esp
			mov ecx,[edi+0x2ec]
			mov ecx,[ecx+0x114]
			add ecx,0x8ec
			push ecx
			push esp
			push edx
			call fixCastFieball
			add esp,8
			mov edx,[esp]
			mov ecx,[esp+4]
			add esp,8 */

			mov ecx,[edi+0x2ec]
			mov ecx,[ecx+0x114]
			sub esp,4
			fild [ecx+0x8ec]
			fsub [edi+0x38]
			fld st //  x-x1
			fmul st,st
			fstp [esp]
			fild [ecx+0x8f0]
			fsub [edi+0x3c]
			fld st //  y-y1
			fmul st,st
			fadd [esp]
			fsqrt 
			fld st
			fdivp st(2),st
			fdivp st(2),st
			fstp [esp] //cos
			mov edx,[esp]
			fstp [esp] //sin 
			mov ecx,[esp]
			add esp,4 
			
l1: // exit
			push 0052C7D9h
			ret
		}
	}
}
extern void InjectOffs(DWORD Addr,void *Fn);
extern void InjectJumpTo(DWORD Addr,void *Fn);

void topicOverrideInit()
{
	//X.D.: What the hell is that? What does it do?
	byte nop[2] = { 0x90, 0x90 };
	//InjectData(0x0040C29B, nop, 2);
}


/// <summary>
/// Checks if player joing packet contains valid data and is the right length.
/// </summary>
/// <param name="packet">Packet data</param>
/// <param name="length">Packet length</param>
bool isPlayerDataPacketValid(char* packet, int length)
{
	if (*packet == 0x20 && length < 0x9A)
		return false;
	packet++;
	//Check if nickname starts with unprintable character
	if (packet[0] < ' ')
		return false;

	// Check if player class is invalid (id is 2 maximum)
	if (packet[0x42] > 2)
		return false;

	// Check if player object requested by client is invalid
	if (packet[0x43] > 0)
		return false;

	packet--;
	return true;
}

int(__cdecl* netOnPacketRecvServ)(int playerId, char* packet, int length);
// Validates incoming player data in order to prevent rogue players crashing the server
int __cdecl netOnPacketRecvServ_Hook(int playerId, char *packet, int length)
{
	//If malformed join request received or character file is malformed - ignore the request.
	if (!isPlayerDataPacketValid(packet, length))
		return 0;
	// Carry on
	return netOnPacketRecvServ(playerId, packet, length);
}

void bugsInit()
{
	ASSIGN(wndSummonUsed,0x00716E88);

	ASSIGN(creatureSummonCommandAll,0x005B4080);

	ASSIGN(sub_4C2BF0,0x004C2BF0);
	ASSIGN(sub_4C3147,0x004C3147);
	ASSIGN(sub_4C31D0,0x004C31D0);

	ASSIGN(wndSummonCreateList,0x004C2560);
	ASSIGN(cliSummondWndLoad,0x004C2E50);

#ifdef FIX_FON_BLOCK_GREATSWORD
	InjectJumpTo(0x004E1C8A,&asmDeathBallBugGs);
	announceCapability("fix_force_of_nature_greatsword_block");
#endif
#ifdef FIX_FON_BLOCK_SHIELD
	InjectJumpTo(0x004E1BD6,&asmDeathBallBugSh);
	announceCapability("fix_force_of_nature_shield_block");
#endif

#ifdef CONJURER_SUMMON_QUICK_CONTROL
	InjectJumpTo(0x004C2BC7,&asmConjSummonAnotherCmp); // support right-clicking on creatures in UI
	InjectJumpTo(0x004C2BB6,&asmConjSummonDo); // Replace list with quick action processor code
	InjectJumpTo(0x0049179A,&asmConjSummonCreate);
	InjectJumpTo(0x004C2ACC,&asmConjSummonDoAll);
	InjectJumpTo(0x004C3147,&asmConjSummonDieOrBanish);
	InjectJumpTo(0x004C1FA1,&asmConjSummonLoadWnd);
	announceCapability("conjurer_summon_quick_commands");
#endif

#ifdef FIX_CAST_FIREBALL
	InjectJumpTo(0x0052C7CD,&asmFixCastFireball);
	announceCapability("fix_cast_fireball");
#endif
	// TODO: convert all similar hardcoded-switches into #defines in a separate file
	
#ifdef FIX_REJECT_MALFORMED_JOIN
	// Validates incoming player data in order to prevent rogue players crashing the server
	ASSIGN(netOnPacketRecvServ, 0x51BAD0);
	InjectOffs(0x4DEC40 + 1, &netOnPacketRecvServ_Hook);
	announceCapability("fix_malformed_character_join");
#endif
}