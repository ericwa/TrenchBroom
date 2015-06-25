/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TrenchBroomApp.h"

#include <wx/docview.h>
#include <wx/menu.h>
#include <wx/intl.h>

#include "Utility/DocManager.h"
#include "View/CommandIds.h"
#include "View/PreferencesFrame.h"

IMPLEMENT_APP(TrenchBroomApp)

BEGIN_EVENT_TABLE(TrenchBroomApp, AbstractApp)
END_EVENT_TABLE()

static wxLocale locale;

wxMenu* TrenchBroomApp::CreateViewMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
	wxMenu* viewMenu = AbstractApp::CreateViewMenu(selector, eventHandler, mapViewFocused);
	viewMenu->AppendSeparator();
    viewMenu->Append(wxID_PREFERENCES, wxT("Preferences..."));
	return viewMenu;
}

wxMenu* TrenchBroomApp::CreateHelpMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
	wxMenu* helpMenu = AbstractApp::CreateHelpMenu(selector, eventHandler, mapViewFocused);
	helpMenu->AppendSeparator();
    helpMenu->Append(wxID_ABOUT, wxT("About TrenchBroom..."));
	return helpMenu;
}

bool TrenchBroomApp::OnInit() {
    // set the locale to US so that we can parse floats property
    locale.Init(wxLANGUAGE_ENGLISH_US);
    
	if (AbstractApp::OnInit()) {
		SetExitOnFrameDelete(true);
		m_docManager->SetUseSDI(true);
        if (wxApp::argc > 1) {
            wxString filename = wxApp::argv[1];
            if (m_docManager->CreateDocument(filename) == NULL) {
                return false;
            }
        } else {
		    m_docManager->CreateNewDocument();
        }
		return true;
	}

	return false;
}
