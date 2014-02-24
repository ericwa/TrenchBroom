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

#include "MiniMapBaseView.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

DEFINE_EVENT_TYPE(EVT_MINIMAP_VIEW_CHANGED_EVENT)

namespace TrenchBroom {
    namespace View {
        MiniMapBaseView::~MiniMapBaseView() {
            delete m_glContext;
            m_glContext = NULL;
        }
        
        void MiniMapBaseView::OnMouseButton(wxMouseEvent& event) {
            if (event.ButtonDown()) {
                if (!HasCapture())
                    CaptureMouse();
                m_lastPos = event.GetPosition();
                SetCursor(wxCursor(wxCURSOR_CLOSED_HAND));
            } else {
                if (HasCapture())
                    ReleaseMouse();
                SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
            }
        }
        
        void MiniMapBaseView::OnMouseDoubleClick(wxMouseEvent& event) {
        }
        
        void MiniMapBaseView::OnMouseMotion(wxMouseEvent& event) {
            if (HasCapture() && event.LeftIsDown()) {
                const wxPoint currentPos = event.GetPosition();
                
                const Vec3f lastWorldPos = camera().unproject(static_cast<float>(m_lastPos.x),
                                                              static_cast<float>(m_lastPos.y),
                                                              0.0f);
                const Vec3f currentWorldPos = camera().unproject(static_cast<float>(currentPos.x),
                                                                 static_cast<float>(currentPos.y),
                                                                 0.0f);
                
                moveCamera(lastWorldPos - currentWorldPos);
                m_lastPos = currentPos;
                Refresh();
            }
        }
        
        void MiniMapBaseView::OnMouseWheel(wxMouseEvent& event) {
            const wxPoint mousePos = event.GetPosition();
            const Vec3f before = camera().unproject(static_cast<float>(mousePos.x),
                                                    static_cast<float>(mousePos.y),
                                                    0.0f);
            
            if (event.GetWheelRotation() > 0)
                zoomCamera(Vec2f(1.1f, 1.1f));
            else
                zoomCamera(Vec2f(1.0f, 1.0f) / 1.1f);
            
            
            const Vec3f after = camera().unproject(static_cast<float>(mousePos.x),
                                                   static_cast<float>(mousePos.y),
                                                   0.0f);
            
            moveCamera(before - after);
            Refresh();
        }
        
        void MiniMapBaseView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
        }
        
        void MiniMapBaseView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    Renderer::RenderContext context(camera(), m_renderResources.shaderManager(), false, 16);
                    setupGL(context);
                    clearBackground(context);
                    renderMap(context);
                }
                SwapBuffers();
            }
#endif
        }
        
        void MiniMapBaseView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            updateViewport(viewport);
            event.Skip();
        }
        
        MiniMapBaseView::MiniMapBaseView(wxWindow* parent, View::MapDocumentWPtr document, BBox3f& bounds, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer) :
        wxGLCanvas(parent, wxID_ANY, &renderResources.glAttribs().front()),
        m_document(document),
        m_bounds(bounds),
        m_renderResources(renderResources),
        m_glContext(new wxGLContext(this, m_renderResources.sharedContext())),
        m_renderer(renderer),
        m_auxVbo(0xFF) {
            SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
            bindEvents();
        }

        View::MapDocumentSPtr MiniMapBaseView::document() const {
            assert(!expired(m_document));
            return lock(m_document);
        }

        void MiniMapBaseView::updateBounds() {
            doUpdateBounds(m_bounds);
            fireChangeEvent();
        }

        void MiniMapBaseView::updateViewport(const Renderer::Camera::Viewport& viewport) {
            doUpdateViewport(viewport);
            updateBounds();
        }
        
        void MiniMapBaseView::moveCamera(const Vec3f& diff) {
            doMoveCamera(diff);
            updateBounds();
        }
        
        void MiniMapBaseView::zoomCamera(const Vec3f& factors) {
            doZoomCamera(factors);
            updateBounds();
        }

        void MiniMapBaseView::bindEvents() {
            Bind(wxEVT_LEFT_DOWN, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_MOTION, &MiniMapBaseView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &MiniMapBaseView::OnMouseWheel, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &MiniMapBaseView::OnMouseCaptureLost, this);
            
            Bind(wxEVT_PAINT, &MiniMapBaseView::OnPaint, this);
            Bind(wxEVT_SIZE, &MiniMapBaseView::OnSize, this);
        }
        
        void MiniMapBaseView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }
        
        void MiniMapBaseView::clearBackground(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        void MiniMapBaseView::renderMap(Renderer::RenderContext& context) {
            m_renderer.render(context, m_bounds);
        }

        void MiniMapBaseView::fireChangeEvent() {
            wxCommandEvent event(EVT_MINIMAP_VIEW_CHANGED_EVENT);
            event.SetEventObject(this);
            event.SetId(GetId());
            ProcessEvent(event);
        }
    }
}