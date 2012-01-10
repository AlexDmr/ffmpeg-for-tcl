// FF_MPEG_DLL.h : fichier d'en-tête principal pour la DLL FF_MPEG_DLL
//

#pragma once

#ifndef __AFXWIN_H__
//	#error "incluez 'stdafx.h' avant d'inclure ce fichier pour PCH"
  #define inline _inline
  #include "stdafx.h"
  #include <windows.h>
#endif

#include "resource.h"		// symboles principaux

#include "stdafx.h"
 
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

// CFF_MPEG_DLLApp
// Consultez FF_MPEG_DLL.cpp pour l'implémentation de cette classe
//
/*
class CFF_MPEG_DLLApp : public CWinApp
{
public:
	CFF_MPEG_DLLApp();

// Substitutions
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
*/