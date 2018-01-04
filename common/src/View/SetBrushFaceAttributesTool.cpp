/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "SetBrushFaceAttributesTool.h"

#include "Model/BrushFace.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(MapDocumentWPtr document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document) {}

        Tool* SetBrushFaceAttributesTool::doGetTool() {
            return this;
        }
        
        bool SetBrushFaceAttributesTool::doMouseClick(const InputState& inputState) {
            return performCopy(inputState, false);
        }
        
        bool SetBrushFaceAttributesTool::doMouseDoubleClick(const InputState& inputState) {
            return performCopy(inputState, true);
        }
        
        bool SetBrushFaceAttributesTool::performCopy(const InputState& inputState, const bool applyToBrush) {
            if (!applies(inputState))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            
            const Model::BrushFaceList& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1)
                return false;
            
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            Model::BrushFace* source = selectedFaces.front();
            Model::BrushFace* targetFace = Model::hitToFace(hit);
            Model::Brush* targetBrush = targetFace->brush();
            
            const Model::WrapStyle wrapStyle = inputState.modifierKeysDown(ModifierKeys::MKShift) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;
            
            const Transaction transaction(document);
            if (copyAttributes(inputState)) {
                if (applyToBrush) {
                    const auto pillarFaces = targetBrush->facesAroundPillar(source, targetFace);
                    const auto otherFaces = SetUtils::minus(SetUtils::makeSet(targetBrush->faces()), SetUtils::makeSet(pillarFaces));
                    
                    {
                        const Model::BrushFace *last = source;
                        for (const auto face : pillarFaces) {
                            // copy from "last" to "face"
                            document->deselectAll();
                            document->select(Model::BrushFaceList{face});
                            
                            Model::TexCoordSystemSnapshot* snapshot = last->takeTexCoordSystemSnapshot();
                            document->setFaceAttributes(last->attribs());
                            if (snapshot != nullptr) {
                                document->copyTexCoordSystemFromFace(snapshot, last->attribs().takeSnapshot(), last->boundary(), wrapStyle);
                                delete snapshot;
                            }
                            
                            last = face;
                        }
                    }
                    
                    // for the other faces, copy from "source" to "face"
                    for (const auto face : otherFaces) {
                        document->deselectAll();
                        document->select(Model::BrushFaceList{face});
                        
                        Model::TexCoordSystemSnapshot* snapshot = source->takeTexCoordSystemSnapshot();
                        document->setFaceAttributes(source->attribs());
                        if (snapshot != nullptr) {
                            document->copyTexCoordSystemFromFace(snapshot, source->attribs().takeSnapshot(), source->boundary(), wrapStyle);
                            delete snapshot;
                        }
                    }
                } else {
                    document->deselectAll();
                    document->select(Model::BrushFaceList{targetFace});
                    
                    Model::TexCoordSystemSnapshot* snapshot = source->takeTexCoordSystemSnapshot();
                    document->setFaceAttributes(source->attribs());
                    if (snapshot != nullptr) {
                        document->copyTexCoordSystemFromFace(snapshot, source->attribs().takeSnapshot(), source->boundary(), wrapStyle);
                        delete snapshot;
                    }
                }
            } else {
                const Model::BrushFaceList targetList = applyToBrush ? targetBrush->faces() : Model::BrushFaceList(1, targetFace);
                document->deselectAll();
                document->select(targetList);
                document->setTexture(source->texture());
            }
            document->deselectAll();
            document->select(source);
            return true;
        }
        
        bool SetBrushFaceAttributesTool::applies(const InputState& inputState) const {
            return inputState.checkModifierKeys(MK_DontCare, MK_Yes, MK_DontCare);
        }

        bool SetBrushFaceAttributesTool::copyAttributes(const InputState& inputState) const {
            return !inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }
        
        bool SetBrushFaceAttributesTool::doCancel() {
            return false;
        }
    }
}
