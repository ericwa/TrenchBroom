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

#ifndef __TrenchBroom__ClipToolAdapter__
#define __TrenchBroom__ClipToolAdapter__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/RenderContext.h"
#include "View/ClipTool.h"
#include "View/InputState.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class ClipTool;
        class Grid;
        class InputState;
        class Tool;
        
        template <typename DragPolicy>
        class ClipToolAdapter : public ToolAdapterBase<PickingPolicy, NoKeyPolicy, MousePolicy, DragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            typedef ToolAdapterBase<PickingPolicy, NoKeyPolicy, MousePolicy, DragPolicy, RenderPolicy, NoDropPolicy> Super;
        protected:
            ClipTool* m_tool;
            const Grid& m_grid;
        protected:
            ClipToolAdapter(ClipTool* tool, const Grid& grid) :
            m_tool(tool),
            m_grid(grid) {}
            
            virtual ~ClipToolAdapter() {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }
            
            bool doMouseClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                    return false;
                return doAddClipPoint(inputState);
            }

            void doPick(const InputState& inputState, Model::PickResult& pickResult) {
                m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
            }
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                renderContext.setHideSelection();
                renderContext.setForceHideSelectionGuide();
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->renderBrushes(renderContext, renderBatch);
                m_tool->renderClipPoints(renderContext, renderBatch);
                m_tool->renderHighlight(Super::dragging(), inputState.pickResult(), renderContext, renderBatch);
            }
            
            bool doCancel() {
                if (m_tool->hasClipPoints()) {
                    m_tool->reset();
                    return true;
                }
                return false;
            }
        protected:
            bool startDrag(const InputState& inputState) {
                if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                    inputState.modifierKeys() != ModifierKeys::MKNone)
                    return false;
                return m_tool->beginDragClipPoint(inputState.pickResult());
            }
        private: // subclassing interface
            virtual bool doAddClipPoint(const InputState& inputState) = 0;
        };
        
        class ClipToolAdapter2D : public ClipToolAdapter<PlaneDragPolicy> {
        public:
            ClipToolAdapter2D(ClipTool* tool, const Grid& grid);
        private:
            class ClipPointSnapper;
            class ClipPointStrategy;
            class ClipPointStrategyFactory;

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            
            bool doAddClipPoint(const InputState& inputState);
        };
        
        class ClipToolAdapter3D : public ClipToolAdapter<MouseDragPolicy> {
        public:
            ClipToolAdapter3D(ClipTool* tool, const Grid& grid);
        private:
            class ClipPointSnapper;
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            bool doAddClipPoint(const InputState& inputState);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipToolAdapter__) */