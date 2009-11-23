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

#import <Cocoa/Cocoa.h>
#include "sysdialog_mac.h"


/// Show a message box.
bool MacMessageBox_Show(const char * aText, const char * aCaption,
  SysMessageBox::MessageType aMessageType)
{
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  // Create the alert object
  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"OK"];
  [alert setMessageText:[NSString stringWithCString:aCaption length:strlen(aCaption)]];
  [alert setInformativeText:[NSString stringWithCString:aText length:strlen(aText)]];
  switch(aMessageType)
  {
    case SysMessageBox::mtInformation:
    default:
      [alert setAlertStyle:NSInformationalAlertStyle];
      break;
    case SysMessageBox::mtWarning:
      [alert setAlertStyle:NSWarningAlertStyle];
      break;
    case SysMessageBox::mtError:
      [alert setAlertStyle:NSCriticalAlertStyle];
      break;
  }

  // Show the dialog
  NSInteger clickedButton = [alert runModal];
  bool result = (clickedButton == NSAlertFirstButtonReturn);

  // Cleanup
  [alert release];
  [pool drain];

  return result;
}
