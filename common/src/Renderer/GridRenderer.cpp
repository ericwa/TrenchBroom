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

#include "GridRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

namespace TrenchBroom {
    namespace Renderer {
        GridRenderer::GridRenderer(const OrthographicCamera& camera) :
        m_vertexArray(VertexArray::copy(GL_QUADS, vertices(camera))) {}

        GridRenderer::Vertex::List GridRenderer::vertices(const OrthographicCamera& camera) {
            Vertex::List result(4);
            
            const Camera::Viewport& viewport = camera.zoomedViewport();
            const float w = float(viewport.width) / 2.0f;
            const float h = float(viewport.height) / 2.0f;

            const Vec3f& p = camera.position();
            switch (camera.direction().firstComponent()) {
                case Math::Axis::AX:
                    result[0] = Vertex(Vec3f(0.0f, p.y() - w, p.z() - h));
                    result[1] = Vertex(Vec3f(0.0f, p.y() - w, p.z() + h));
                    result[2] = Vertex(Vec3f(0.0f, p.y() + w, p.z() + h));
                    result[3] = Vertex(Vec3f(0.0f, p.y() + w, p.z() - h));
                    break;
                case Math::Axis::AY:
                    result[0] = Vertex(Vec3f(p.x() - w, 0.0f, p.z() - h));
                    result[1] = Vertex(Vec3f(p.x() - w, 0.0f, p.z() + h));
                    result[2] = Vertex(Vec3f(p.x() + w, 0.0f, p.z() + h));
                    result[3] = Vertex(Vec3f(p.x() + w, 0.0f, p.z() - h));
                    break;
                case Math::Axis::AZ:
                    result[0] = Vertex(Vec3f(p.x() - w, p.y() - h, 0.0f));
                    result[1] = Vertex(Vec3f(p.x() - w, p.y() + h, 0.0f));
                    result[2] = Vertex(Vec3f(p.x() + w, p.y() + h, 0.0f));
                    result[3] = Vertex(Vec3f(p.x() + w, p.y() - h, 0.0f));
                    break;
            }

            return result;
        }

        void GridRenderer::doPrepare(Vbo& vbo) {
            m_vertexArray.prepare(vbo);
        }
        
        void GridRenderer::doRender(RenderContext& renderContext) {
            const Camera& camera = renderContext.camera();
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::Grid2DShader);
            shader.set("Normal", -camera.direction());
            shader.set("RenderGrid", renderContext.showGrid());
            shader.set("GridSize", static_cast<float>(renderContext.gridSize()));
            shader.set("GridAlpha", pref(Preferences::GridAlpha));
            shader.set("CameraZoom", renderContext.camera().zoom());
            
            m_vertexArray.render();
        }
    }
}