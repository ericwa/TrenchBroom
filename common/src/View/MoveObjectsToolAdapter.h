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

#ifndef __TrenchBroom__MoveObjectsToolAdapter__
#define __TrenchBroom__MoveObjectsToolAdapter__

#include "View/MoveToolAdapter.h"

namespace TrenchBroom {
    namespace View {
        class MoveObjectsTool;
        
        class MoveObjectsToolAdapter : public MoveToolAdapter<NoPickingPolicy, NoMousePolicy, RenderPolicy> {
        private:
            MoveObjectsTool* m_tool;
        public:
            MoveObjectsToolAdapter(MoveObjectsTool* tool, MoveToolHelper* helper);
            virtual ~MoveObjectsToolAdapter();
        private:
            Tool* doGetTool();

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        };
        
        class MoveObjectsToolAdapter2D : public MoveObjectsToolAdapter {
        public:
            MoveObjectsToolAdapter2D(MoveObjectsTool* tool);
        };
        
        class MoveObjectsToolAdapter3D : public MoveObjectsToolAdapter {
        public:
            MoveObjectsToolAdapter3D(MoveObjectsTool* tool, MovementRestriction& movementRestriction);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsToolAdapter__) */