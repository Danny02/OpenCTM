//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        sysdialog.cpp
// Description: Implementation of system GUI dialog routines.
//-----------------------------------------------------------------------------
// Copyright (c) 2009 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//-----------------------------------------------------------------------------

// What system are we on?
#if !defined(WIN32) && defined(_WIN32)
#define WIN32
#endif

#include <cstring>

#ifdef WIN32
#include <windows.h>
#else
#include <iostream>
#endif

#include "sysdialog.h"

using namespace std;


/// Constructor.
SysMessageBox::SysMessageBox()
{
  mMessageType = mtInformation;
}

/// Show the dialog.
bool SysMessageBox::Show()
{
#ifdef WIN32
  DWORD dialogType;
  switch(mMessageType)
  {
    default:
    case mtInformation:
      dialogType = MB_ICONINFORMATION;
      break;
    case mtWarning:
      dialogType = MB_ICONWARNING;
      break;
    case mtError:
      dialogType = MB_ICONERROR;
      break;
  }
  MessageBoxA(NULL, mText.c_str(), mCaption.c_str(), MB_OK | dialogType);
  return true;
#else
  cout << mCaption << ": " << mText << endl;
  return true;
#endif
}


/// Convenience function for the message box.
void ShowMessage(const char * aMessage, const char * aCaption)
{
  SysMessageBox mb;
  mb.mCaption = aCaption;
  mb.mText = aMessage;
  mb.Show();
}


/// Constructor
SysOpenDialog::SysOpenDialog()
{
  mMultiSelect = false;
}

/// Show the dialog.
bool SysOpenDialog::Show()
{
#ifdef WIN32
  OPENFILENAME ofn;
  char fileNameBuf[10000];

  // Initialize the file dialog structure
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.lpstrFilter = NULL; // FIXME
  ofn.nFilterIndex = 1;
  memset(&fileNameBuf, 0, sizeof(fileNameBuf));
  mFileName.copy(fileNameBuf, mFileName.size());
  ofn.lpstrFile = fileNameBuf;
  ofn.nMaxFile = sizeof(fileNameBuf);
  ofn.lpstrTitle = mCaption.c_str();
  ofn.Flags = 0;

  // Show the save dialog
  bool result = GetOpenFileNameA(&ofn);

  // Extract the resulting file names
  mFileNames.clear();
  mFileName = string("");
  if(result)
  {
    unsigned int pos = 0;
    while((pos < sizeof(fileNameBuf)) && (fileNameBuf[pos] != 0))
    {
      unsigned int start = pos;
      while((pos < sizeof(fileNameBuf)) && (fileNameBuf[pos] != 0))
        ++ pos;
      if(pos < sizeof(fileNameBuf))
      {
        mFileNames.push_back(string(&fileNameBuf[start]));
        ++ pos;
      }
    }
    if(mFileNames.size() > 0)
      mFileName = *(mFileNames.begin());
  }

  return result;
#else
  cout << "SysOpenDialog is not yet implemented for your system." << endl;
  return false;
#endif
}


/// Constructor
SysSaveDialog::SysSaveDialog()
{
}

/// Show the dialog.
bool SysSaveDialog::Show()
{
#ifdef WIN32
  OPENFILENAME ofn;
  char fileNameBuf[1000];

  // Initialize the file dialog structure
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.lpstrFilter = NULL; // FIXME
  ofn.nFilterIndex = 1;
  memset(&fileNameBuf, 0, sizeof(fileNameBuf));
  mFileName.copy(fileNameBuf, mFileName.size());
  ofn.lpstrFile = fileNameBuf;
  ofn.nMaxFile = sizeof(fileNameBuf);
  ofn.lpstrTitle = mCaption.c_str();
  ofn.Flags = 0;

  // Show the save dialog
  bool result = GetSaveFileNameA(&ofn);

  // Extract the resulting file name
  if(result)
    mFileName = string(fileNameBuf);
  else
    mFileName = string("");

  return result;
#else
  cout << "SysSaveDialog is not yet implemented for your system." << endl;
  return false;
#endif
}
