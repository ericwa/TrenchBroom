/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__KeyboardShortcutEntry__
#define __TrenchBroom__KeyboardShortcutEntry__

#include <wx/wx.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        class KeyboardShortcutEntry {
        public:
            typedef std::vector<KeyboardShortcutEntry*> List;
        private:
            bool m_hasConflicts;
        protected:
            KeyboardShortcutEntry();
        public:
            virtual ~KeyboardShortcutEntry();
            
            bool modifiable() const;
            int requiredModifiers() const;
            
            bool hasConflicts() const;
            void resetConflicts();
            bool updateConflicts(const KeyboardShortcutEntry* entry);
        private:
            bool conflictsWith(const KeyboardShortcutEntry* entry) const;
        public:
            int actionContext() const;
            wxString actionContextDescription() const;
            
            wxString actionDescription() const;
            wxString shortcutDescription() const;
            
            bool equals(const KeyboardShortcut& shortcut) const;
            void updateShortcut(const KeyboardShortcut& shortcut);
        private:
            virtual int doGetActionContext() const = 0;
            virtual bool doGetModifiable() const = 0;
            virtual int doGetRequiredModifiers() const = 0;
            virtual wxString doGetActionDescription() const = 0;
            virtual const KeyboardShortcut& doGetShortcut() const = 0;
            virtual void doUpdateShortcut(const KeyboardShortcut& shortcut) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardShortcutEntry__) */