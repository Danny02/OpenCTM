//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        sysdialog_mac.mm
// Description: Implementation of system GUI dialog routines for Mac OS X, 
//              using the Objective-C based COCOA API.
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

#include <NSAlert.h>
#include "sysdialog_mac.h"


/// Show a message box.
bool MacMessageBox_Show(const char * aText, const char * aCaption,
  SysMessageBox::MessageType aMessageType)
{
  // Create the alert object
  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"OK"];
  [alert setMessageText:@aCaption];
  [alert setInformativeText:@aText];
  switch(aMessageType)
  {
    case SysDialogBox::mtInformation:
    default:
      [alert setAlertStyle:NSInformationalAlertStyle];
      break;
    case SysDialogBox::mtWarning:
      [alert setAlertStyle:NSWarningAlertStyle];
      break;
    case SysDialogBox::mtError:
      [alert setAlertStyle:NSCriticalAlertStyle];
      break;
  }

  // Show the dialog
  bool result = ([alert runModal] == NSAlertFirstButtonReturn);

  // Free the alert
  [alert release];

  return result;
}
