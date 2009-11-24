//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        sysdialog_win.cpp
// Description: Implementation of system GUI dialog routines for Windows.
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

#include <windows.h>
#include <cstring>
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
  // Select message type
  DWORD messageType;
  switch(mMessageType)
  {
    default:
    case mtInformation:
      messageType = MB_ICONINFORMATION;
      break;
    case mtWarning:
      messageType = MB_ICONWARNING;
      break;
    case mtError:
      messageType = MB_ICONERROR;
      break;
  }

  // Show the message box
  MessageBoxA(NULL, mText.c_str(), mCaption.c_str(), MB_OK | messageType);

  return true;
}


/// Constructor
SysOpenDialog::SysOpenDialog()
{
  mCaption = "Open File";
}

/// Show the dialog.
bool SysOpenDialog::Show()
{
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

  // Show the dialog
  bool result = GetOpenFileNameA(&ofn);

  // Extract the resulting file name
  if(result)
    mFileName = string(fileNameBuf);
  else
    mFileName = string("");

  return result;
}


/// Constructor
SysSaveDialog::SysSaveDialog()
{
  mCaption = "Save File";
}

/// Show the dialog.
bool SysSaveDialog::Show()
{
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

  // Show the dialog
  bool result = GetSaveFileNameA(&ofn);

  // Extract the resulting file name
  if(result)
    mFileName = string(fileNameBuf);
  else
    mFileName = string("");

  return result;
}
