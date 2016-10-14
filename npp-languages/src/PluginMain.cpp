// This file is part of Extra Languages for Notepad++.
// Copyright (C) 2016 William Newbery <wnewbery@hotmail.co.uk>
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

//#define SCI_NAMESPACE  Notepad++ does not support this!
#include <Windows.h>

#include <string> //must include before ExternalLexer.h
#include <ILexer.h> //must include before LexerModule.h
#include <LexerModule.h> //must include before ExternalLexer.h
#include <Platform.h> //must include before ExternalLexer.h
#include <ExternalLexer.h>

#include <MISC/PluginsManager/PluginInterface.h>
//#include <MISC/PluginsManager/PluginsManager.h>

#include "BaseLexer.h"
#include "lexers/Haml.h"

namespace
{
	void cmdAbout();

	template<size_t N>
	FuncItem createNppCommand(const wchar_t (&name)[N], PFUNCPLUGINCMD func)
	{
		FuncItem ret;
		static_assert(
			N < sizeof(ret._itemName) / sizeof(ret._itemName[0]),
			"Notepad++ command name is too long");
		memcpy(ret._itemName, name, sizeof(name));
		ret._pFunc = func;
		ret._cmdID = 0;
		ret._init2Check = false;
		ret._pShKey = nullptr;
		return ret;
	}
	static const FuncItem NPP_CMD_MENU[] = {
		createNppCommand(L"About", cmdAbout)
	};
	static const auto NPP_CMD_MENU_CNT = sizeof(NPP_CMD_MENU) / sizeof(NPP_CMD_MENU[0]);
	static const LexerInfo LEXERS[] = {
		{"Haml", L"Haml", lexerFactory<Haml>}
	};
	static const auto LEXER_CNT = sizeof(LEXERS) / sizeof(LEXERS[0]);

	void init()
	{
	}
	void shutdown()
	{
	}
	void cmdAbout()
	{
		MessageBoxW(NULL,
			L"A collection of additional or enhanced web development related languages.",
			L"About Extra Notepad++ Languages",
			MB_OK);
	}
}

// Win32 DLL API
BOOL APIENTRY DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		init();
		break;
	case DLL_PROCESS_DETACH:
		shutdown();
		break;
	}
	return TRUE;
}

// Plugin API
extern "C" __declspec(dllexport) void setInfo(NppData data)
{
}
extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return L"Extra Languages";
}
extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *outCount)
{
	*outCount = static_cast<int>(NPP_CMD_MENU_CNT);
	return const_cast<FuncItem*>(NPP_CMD_MENU);
}
extern "C" __declspec(dllexport) void beNotified(SCNotification *)
{
}
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}

// Lexer API
extern "C" _declspec(dllexport) int GetLexerCount()
{
	return (int)LEXER_CNT;
}
extern "C" _declspec(dllexport) void GetLexerName(unsigned int index, char *name, int buflength)
{
	strcpy_s(name, (size_t)buflength, LEXERS[index].name);
}
extern "C" _declspec(dllexport) LexerFactoryFunction GetLexerFactory(unsigned index)
{
	return LEXERS[index].factory;
}
extern "C" _declspec(dllexport) void GetLexerStatusText(unsigned int index, TCHAR *desc, int buflength)
{
	wcscpy_s(desc, (size_t)buflength, LEXERS[index].desc);
}
