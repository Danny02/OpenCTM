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
#elif defined(USE_GTK)
#include <gtk/gtk.h>
#endif

#include <iostream>

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

#elif defined(USE_GTK)

  // Init GTK+
  if(!gtk_init_check(0, NULL))
    return true;

  GtkMessageType dialogType;
  switch(mMessageType)
  {
    default:
    case mtInformation:
      dialogType = GTK_MESSAGE_INFO;
      break;
    case mtWarning:
      dialogType = GTK_MESSAGE_WARNING;
      break;
    case mtError:
      dialogType = GTK_MESSAGE_ERROR;
      break;
  }

  // Create dialog widget
  GtkWidget * dialog = gtk_message_dialog_new(
    NULL,
    GTK_DIALOG_DESTROY_WITH_PARENT,
    dialogType,
    GTK_BUTTONS_OK,
    mText.c_str(), "title");
  gtk_window_set_title(GTK_WINDOW(dialog), mCaption.c_str());

  // Execute dialog
  gint dlgResult = gtk_dialog_run(GTK_DIALOG(dialog));

  // Free the dialog widget (we're done with it)
  gtk_widget_destroy(dialog);
  while(gtk_events_pending()) gtk_main_iteration();

  // Evaluate dialog result
  return (dlgResult == GTK_RESPONSE_ACCEPT);

#else

  cout << mCaption << ": " << mText << endl;
  return true;

#endif
}


/// Constructor
SysOpenDialog::SysOpenDialog()
{
  mCaption = "Open File";
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
  if(result)
    mFileName = string(fileNameBuf);
  else
    mFileName = string("");

  return result;

#elif defined(USE_GTK)

  // Init GTK+
  if(!gtk_init_check(0, NULL))
    return true;

  // Create dialog widget
  GtkWidget * dialog = gtk_file_chooser_dialog_new(
    mCaption.c_str(),
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    NULL);

  // Add filters
  for(list<string>::iterator i = mFilters.begin(); i != mFilters.end(); ++ i)
  {
    size_t splitPos = (*i).find("|");
    if(splitPos != string::npos)
    {
      string name = (*i).substr(0, splitPos);
      string pattern = (*i).substr(splitPos + 1);
      GtkFileFilter * filter = gtk_file_filter_new();
      gtk_file_filter_set_name(filter, name.c_str());
      size_t pos1 = 0;
      while(pos1 != string::npos)
      {
        size_t pos2 = pattern.find(";", pos1);
        gtk_file_filter_add_pattern(filter, (pattern.substr(pos1, pos2 - pos1)).c_str());
        if(pos2 != string::npos)
          pos1 = pos2 + 1;
        else
          pos1 = pos2;
      }
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    }
  }

  // Execute dialog
  gint dlgResult = gtk_dialog_run(GTK_DIALOG(dialog));

  // Collect file name
  if(dlgResult == GTK_RESPONSE_ACCEPT)
  {
    char * fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    mFileName = string(fileName);
    g_free(fileName);
  }

  // Free the dialog widget (we're done with it)
  gtk_widget_destroy(dialog);
  while(gtk_events_pending()) gtk_main_iteration();

  // Evaluate dialog result
  return (dlgResult == GTK_RESPONSE_ACCEPT);

#else

  cout << "SysOpenDialog is not yet implemented for your system." << endl;
  return false;

#endif
}


/// Constructor
SysSaveDialog::SysSaveDialog()
{
  mCaption = "Save File";
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

#elif defined(USE_GTK)

  // Init GTK+
  if(!gtk_init_check(0, NULL))
    return true;

  // Create dialog widget
  GtkWidget * dialog = gtk_file_chooser_dialog_new(
    mCaption.c_str(),
    NULL,
    GTK_FILE_CHOOSER_ACTION_SAVE,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
    NULL);

  // Add filters
  for(list<string>::iterator i = mFilters.begin(); i != mFilters.end(); ++ i)
  {
    size_t splitPos = (*i).find("|");
    if(splitPos != string::npos)
    {
      string name = (*i).substr(0, splitPos);
      string pattern = (*i).substr(splitPos + 1);
      GtkFileFilter * filter = gtk_file_filter_new();
      gtk_file_filter_set_name(filter, name.c_str());
      size_t pos1 = 0;
      while(pos1 != string::npos)
      {
        size_t pos2 = pattern.find(";", pos1);
        gtk_file_filter_add_pattern(filter, (pattern.substr(pos1, pos2 - pos1)).c_str());
        if(pos2 != string::npos)
          pos1 = pos2 + 1;
        else
          pos1 = pos2;
      }
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    }
  }

  // Execute dialog
  gint dlgResult = gtk_dialog_run(GTK_DIALOG(dialog));

  // Collect file name
  if(dlgResult == GTK_RESPONSE_ACCEPT)
  {
    char * fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    mFileName = string(fileName);
    g_free(fileName);
  }

  // Free the dialog widget (we're done with it)
  gtk_widget_destroy(dialog);
  while(gtk_events_pending()) gtk_main_iteration();

  // Evaluate dialog result
  return (dlgResult == GTK_RESPONSE_ACCEPT);

#else

  cout << "SysSaveDialog is not yet implemented for your system." << endl;
  return false;

#endif
}
