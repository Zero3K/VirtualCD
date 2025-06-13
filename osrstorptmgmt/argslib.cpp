//***********************************************************************
// (c) Copyright 1999-2003 Santronics Software, Inc. All Rights Reserved.
//***********************************************************************
// File Name : argslib.cpp
// Subsystem : Command Line Parameters Wrapper
// Date      : 03/03/2003
// Author    : Hector Santos, Santronics Software, Inc.
// VERSION   : 1.00P
//
// Revision History:
// Version  Date      Author  Comments
// -------  --------  ------  -------------------------------------------
// v1.00P   03/03/03  HLS     Public Release version (non-SSL version)
//***********************************************************************
#include "stdafx.h"
#include "argslib.h"


///////////////////////////////////////////////////////////////
// cut/paste example usage:
/*
    #include "argslib.h"


    CCommandLineParameters clp;
    if (clp.CheckHelp(FALSE) {  // set TRUE to display help if no switches
        ... display help ...
        return;
    }
    CString sServer = clp.GetSwitchStr("server");
    if (clp.Switch("debug")) DebugLogging     = TRUE;
    if (clp.Switch("dlog+")) DailyLogsEnabled = TRUE;
    if (clp.Switch("dlog-")) DailyLogsEnabled = FALSE;
    if (clp.Switch("trace")) EnableTraceLog   = TRUE;

    if (sServer.IsEmpty()) {
        if (!WildcatServerConnect(NULL)) {
            return FALSE;
        }
    } else {
        if (!WildcatServerConnectSpecific(NULL,sServer)) {
            return FALSE;
        }
    }
*/
///////////////////////////////////////////////////////////////

//-------------------------------------------------------------
// CreateParameterFromString()
//
// Given command line string, generate array of strings

int CreateParameterFromString(TCHAR *pszParams, TCHAR *argv[], int max)
{
    int argc = 0;
    if (pszParams) {
        TCHAR *p = pszParams;
        while (*p && (argc < max)) {
            while (*p == _T(' ')) {
                p++;
            }
            if (!*p) {
                break;
            }
            if (*p == _T('"')) {
                p++;
                argv[argc++] = p;
                while (*p && *p != _T('"')) {
                    p++;
                }
            } else {
                argv[argc++] = p;
                while (*p && *p != _T(' ')) {
                    p++;
                }
            }
            if (*p) {
                *p++ = 0;
            }
        }
    }
    return argc;
}

CCommandLineParameters::CCommandLineParameters(TCHAR *switchchars /* = "-/" */)
    : szSwitchChars(switchchars)
{
    int maxparms = 100;
    pszCmdLineDup = _tcsdup(GetCommandLine());
    paramcount = CreateParameterFromString(pszCmdLineDup,parms,maxparms);
}

CCommandLineParameters::~CCommandLineParameters()
{
    if (pszCmdLineDup) {
      free(pszCmdLineDup);
      pszCmdLineDup = NULL;
    }
}

BOOL CCommandLineParameters::CheckHelp(const BOOL bNoSwitches /*= FALSE */)
{
     if (bNoSwitches && (paramcount < 2)) return TRUE;
     if (paramcount < 2) return FALSE;
     if (_tcscmp(ParamStr(1),_T("-?")) == 0) return TRUE;
     if (_tcscmp(ParamStr(1),_T("/?")) == 0) return TRUE;
     if (_tcscmp(ParamStr(1),_T("/help")) == 0) return TRUE;
     if (_tcscmp(ParamStr(1),_T("?")) == 0) return TRUE;
     return FALSE;
}

CString CCommandLineParameters::ParamStr(const int index, const BOOL bGetAll /* = FALSE */)
{
    if ((index < 0) || (index >= paramcount)) {
        return _T("");
    }
    CString s = parms[index];
    if (bGetAll) {
        for (int i = index+1;i < paramcount; i++) {
              s += _T(" ");
              s += parms[i];
        }
    }
    return s;
}

int CCommandLineParameters::ParamInt(const int index)
{
    return _ttoi(ParamStr(index));
}

CString CCommandLineParameters::ParamLine()
{
    CString s;
    TCHAR *p = _tcschr(GetCommandLine(),_T(' '));
    if (p) {
        s.Format(_T("%s"),p+1);
    }
    return s;
}

CString CCommandLineParameters::CommandLine()
{
    CString s;
    s.Format(_T("%s"),GetCommandLine());
    return s;
}

BOOL CCommandLineParameters::IsSwitch(TCHAR *sz)
{
    return (_tcschr(szSwitchChars,sz[0]) != NULL);
}


int CCommandLineParameters::SwitchCount()
{
    int count = 0;
    for (int i = 1;i < paramcount; i++) {
        if (IsSwitch(parms[i])) count++;
    }
    return count;
}

int CCommandLineParameters::FirstNonSwitchIndex()
{
    for (int i = 1;i < paramcount; i++) {
        if (!IsSwitch(parms[i])) {
            return i;
        }
    }
    return 0;
}

CString CCommandLineParameters::FirstNonSwitchStr()     // 499.5 04/16/01 12:17 am
{
    // returns the first non-switch, handles lines such as:
    // [options] file [specs]
    return GetNonSwitchStr(FALSE,TRUE);
}

//////////////////////////////////////////////////////////////////////////
// Switch() will return the parameter index if the switch exist.
// Return 0 if not found.  The logic will allow for two types of
// switches:
//
//          /switch value
//          /switch:value
//
// DO NOT PASS THE COLON. IT IS AUTOMATICALLY CHECKED.  In other
// words, the following statements are the same:
//
//         Switch("server");
//         Switch("-server");
//         Switch("/server");
//
// to handle the possible arguments:
//
//         /server:value
//         /server value
//         -server:value
//         -server value
//

int CCommandLineParameters::Switch(TCHAR *sz,
                                   const BOOL bCase /* = FALSE */
                                   )
{
    if (!sz || !sz[0]) {
        return 0;
    }

    TCHAR sz2[255];
    _tcsncpy(sz2,sz,(sizeof(sz2)-sizeof(TCHAR))/sizeof(TCHAR));
    sz2[(sizeof(sz2)-sizeof(TCHAR))/sizeof(TCHAR)] = 0;

    TCHAR *p = sz2;
    if (_tcschr(szSwitchChars,*p) != NULL) p++;

    // check for abbrevation

    int amt = 0;
    TCHAR *abbr = _tcschr(p,_T('*'));
    if (abbr) {
        *abbr = 0;
        amt = _tcslen(p);
        _tcscpy(abbr,abbr+1);
    }

    for (int i = 1;i < paramcount; i++) {
      if (!IsSwitch(parms[i])) continue;
      TCHAR *pColon = _tcschr(&parms[i][1],_T(':'));
      if (pColon && (amt == 0)) { amt = _tcslen(p); }

      if (bCase) {
        if (amt > 0) {
          if (_tcsncmp(p,&parms[i][1],_tcslen(p)) != 0) continue; // 450.6b20
          if (_tcsncmp(p,&parms[i][1],amt) == 0) return i;
        } else {
          if (_tcscmp(p,&parms[i][1]) == 0) return i;
        }
      } else {
        if (amt > 0) {
          if (_tcsnicmp(p,&parms[i][1],_tcslen(p)) != 0) continue; // 450.6b20
          if (_tcsnicmp(p,&parms[i][1],amt) == 0) return i;
        } else {
          if (_tcsicmp(p,&parms[i][1]) == 0) return i;
        }
      }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// GetSwitchStr() will return the string for the given switch. The logic
// will allow for two types of switches:
//
//   /switch value
//   /switch:value
//

CString CCommandLineParameters::GetSwitchStr(TCHAR *sz,
                                             TCHAR *szDefault, /* = "" */
                                             const BOOL bCase /* = FALSE */
                                            )
{
    int idx = Switch(sz,bCase);
    if (idx > 0) {
        CString s = ParamStr(idx);
        int n = s.Find(_T(':'));
        if (n > -1) {
            return s.Mid(n+1);
        } else {
          if ((idx+1) < paramcount) {
              if (!IsSwitch(parms[idx+1])) {
                  return parms[idx+1];
              }
          }
        }
        //return szDefault;
    }
    return szDefault;
}

int CCommandLineParameters::GetSwitchInt(TCHAR *sz,
                                          const int iDefault, /* = 0 */
                                          const BOOL bCase /* = FALSE */
                                         )
{
    TCHAR szDefault[25];
    _stprintf(szDefault,_T("%d"),iDefault);
    return _ttoi(GetSwitchStr(sz,szDefault,bCase));
}

CString CCommandLineParameters::GetNonSwitchStr(
                                const BOOL bBreakAtSwitch, /* = TRUE */
                                const BOOL bFirstOnly /* = FALSE */)
{
    CString sLine = _T("");
    int i = 1;
    while (i < paramcount) {
        if (IsSwitch(parms[i])) {
            if (bBreakAtSwitch) break;
        } else {
            sLine += parms[i];
            if (bFirstOnly) break;
            sLine += _T(" ");
        }
        i++;
    }
    sLine.TrimRight();
    return sLine;
}


