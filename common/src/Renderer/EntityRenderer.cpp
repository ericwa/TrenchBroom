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

#include "EntityRenderer.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityRenderer::EntityClassnameAnchor::EntityClassnameAnchor(const Model::Entity* entity) :
        m_entity(entity) {}
        
        Vec3f EntityRenderer::EntityClassnameAnchor::basePosition() const {
            const Assets::EntityModel* model = m_entity->model();
            Vec3f position = m_entity->bounds().center();
            position[2] = float(m_entity->bounds().max.z());
            if (model != NULL) {
                const Assets::ModelSpecification spec = m_entity->modelSpecification();
                const BBox3f modelBounds = model->bounds(spec.skinIndex, spec.frameIndex);
                const Vec3f origin = m_entity->origin();
                position[2] = std::max(position[2], modelBounds.max.z() + origin.z());
            }
            position[2] += 2.0f;
            return position;
        }
        
        Alignment::Type EntityRenderer::EntityClassnameAnchor::alignment() const {
            return Alignment::Bottom;
        }

        EntityRenderer::EntityClassnameFilter::EntityClassnameFilter(const Model::EditorContext& editorContext, const bool showHiddenEntities) :
        m_editorContext(editorContext),
        m_showHiddenEntities(showHiddenEntities) {}

        bool EntityRenderer::EntityClassnameFilter::stringVisible(RenderContext& renderContext, const Key& entity) const {
            return m_showHiddenEntities || m_editorContext.visible(entity);
        }

        EntityRenderer::EntityClassnameColorProvider::EntityClassnameColorProvider(const Color& textColor, const Color& backgroundColor) :
        m_textColor(textColor),
        m_backgroundColor(backgroundColor) {}

        Color EntityRenderer::EntityClassnameColorProvider::textColor(RenderContext& renderContext, const Key& entity) const {
            return m_textColor;
        }
        
        Color EntityRenderer::EntityClassnameColorProvider::backgroundColor(RenderContext& renderContext, const Key& entity) const {
            return m_backgroundColor;
        }

        EntityRenderer::EntityRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext) :
        m_editorContext(editorContext),
        m_classnameRenderer(ClassnameRenderer(font())),
        m_modelRenderer(entityModelManager, m_editorContext),
        m_boundsValid(false),
        m_showOccludedOverlays(false),
        m_tint(false),
        m_overrideBoundsColor(false),
        m_showOccludedBounds(false),
        m_showAngles(false),
        m_showHiddenEntities(false),
        m_vbo(0xFFF) {
            m_classnameRenderer.setFadeDistance(500.0f);
        }
        
        EntityRenderer::~EntityRenderer() {
            clear();
        }
        
        void EntityRenderer::addEntity(Model::Entity* entity) {
            assert(entity != NULL);

            assert(m_entities.count(entity) == 0);
            m_entities.insert(entity);
            m_classnameRenderer.addString(entity, entityString(entity), TextAnchor::Ptr(new EntityClassnameAnchor(entity)));
            m_modelRenderer.addEntity(entity);
            
            invalidateBounds();
        }
        
        void EntityRenderer::updateEntity(Model::Entity* entity) {
            assert(entity != NULL);
            
            assert(m_entities.count(entity) == 1);
            m_classnameRenderer.updateString(entity, entityString(entity));
            m_modelRenderer.updateEntity(entity);
            invalidateBounds();
        }
        
        void EntityRenderer::removeEntity(Model::Entity* entity) {
            assert(entity != NULL);
            
            Model::EntitySet::iterator it = m_entities.find(entity);
            assert(it != m_entities.end());
            m_entities.erase(it);
            
            m_classnameRenderer.removeString(entity);
            m_modelRenderer.removeEntity(entity);
            invalidateBounds();
        }
        
        void EntityRenderer::invalidate() {
            invalidateBounds();
        }

        void EntityRenderer::clear() {
            m_entities.clear();
            m_wireframeBoundsRenderer = EdgeRenderer();
            m_solidBoundsRenderer = TriangleRenderer();
            m_classnameRenderer.clear();
            m_modelRenderer.clear();
        }

        void EntityRenderer::reloadModels() {
            m_modelRenderer.clear();
            m_modelRenderer.addEntities(m_entities.begin(), m_entities.end());
        }

        void EntityRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_overlayTextColor = overlayTextColor;
        }
        
        void EntityRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_overlayBackgroundColor = overlayBackgroundColor;
        }
        
        void EntityRenderer::setShowOccludedOverlays(const bool showOccludedOverlays) {
            m_showOccludedOverlays = showOccludedOverlays;
        }
        
        void EntityRenderer::setTint(const bool tint) {
            m_tint = tint;
        }
        
        void EntityRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }
        
        void EntityRenderer::setOverrideBoundsColor(const bool overrideBoundsColor) {
            m_overrideBoundsColor = overrideBoundsColor;
        }
        
        void EntityRenderer::setBoundsColor(const Color& boundsColor) {
            m_boundsColor = boundsColor;
        }
        
        void EntityRenderer::setShowOccludedBounds(const bool showOccludedBounds) {
            m_showOccludedBounds = showOccludedBounds;
        }
        
        void EntityRenderer::setOccludedBoundsColor(const Color& occludedBoundsColor) {
            m_occludedBoundsColor = occludedBoundsColor;
        }
        
        void EntityRenderer::setShowAngles(const bool showAngles) {
            m_showAngles = showAngles;
        }
        
        void EntityRenderer::setAngleColor(const Color& angleColor) {
            m_angleColor = angleColor;
        }
        
        void EntityRenderer::setShowHiddenEntities(const bool showHiddenEntities) {
            m_showHiddenEntities = showHiddenEntities;
        }

        void EntityRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            renderBounds(renderContext, renderBatch);
            renderModels(renderContext, renderBatch);
            renderClassnames(renderContext, renderBatch);
            renderAngles(renderContext, renderBatch);
        }
        
        void EntityRenderer::renderBounds(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_boundsValid)
                validateBounds();

            if (renderContext.showEntityBounds())
                renderWireframeBounds(renderBatch);
            if (m_showHiddenEntities || renderContext.showPointEntities())
                renderSolidBounds(renderBatch);
        }
        
        void EntityRenderer::renderWireframeBounds(RenderBatch& renderBatch) {
            if (m_showOccludedBounds)
                renderBatch.addOneShot(new RenderOccludedEdges(m_wireframeBoundsRenderer, m_overrideBoundsColor, m_boundsColor));
            renderBatch.addOneShot(new RenderUnoccludedEdges(m_wireframeBoundsRenderer, m_overrideBoundsColor, m_boundsColor));
        }
        
        void EntityRenderer::renderSolidBounds(RenderBatch& renderBatch) {
            m_solidBoundsRenderer.setApplyTinting(m_tint);
            m_solidBoundsRenderer.setTintColor(m_tintColor);
            renderBatch.add(&m_solidBoundsRenderer);
        }
        
        void EntityRenderer::renderModels(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (m_showHiddenEntities || (renderContext.showPointEntities() &&
                                         renderContext.showPointEntityModels())) {
                m_modelRenderer.setApplyTinting(m_tint);
                m_modelRenderer.setTintColor(m_tintColor);
                m_modelRenderer.setShowHiddenEntities(m_showHiddenEntities);
                m_modelRenderer.render(renderBatch);
            }
        }
        
        void EntityRenderer::renderClassnames(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (renderContext.showEntityClassnames()) {
                if (m_showOccludedOverlays)
                    glDisable(GL_DEPTH_TEST);
                EntityClassnameFilter textFilter(m_editorContext, m_showHiddenEntities);
                EntityClassnameColorProvider colorProvider(m_overlayTextColor, m_overlayBackgroundColor);
                m_classnameRenderer.render(renderContext, renderBatch, textFilter, colorProvider,
                                           Shaders::ColoredTextShader, Shaders::TextBackgroundShader);
                if (m_showOccludedOverlays)
                    glEnable(GL_DEPTH_TEST);
            }
        }
        
        void EntityRenderer::renderAngles(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_showAngles)
                return;
            
            static const float maxDistance2 = 500.0f * 500.0f;
            typedef VertexSpecs::P3::Vertex Vertex;
            const Vec3f::List arrow = arrowHead(9.0f, 6.0f);
            
            Vertex::List vertices;
            Model::EntitySet::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (!m_showHiddenEntities && !m_editorContext.visible(entity))
                    continue;
                
                const Quatf rotation = Quatf(entity->rotation());
                const Vec3f direction = rotation * Vec3f::PosX;
                const Vec3f center = Vec3f(entity->bounds().center());
                
                const Vec3f toCam = renderContext.camera().position() - center;
                if (toCam.squaredLength() > maxDistance2)
                    continue;

                Vec3f onPlane = toCam - toCam.dot(direction) * direction;
                if (onPlane.null())
                    continue;
                
                onPlane.normalize();
                
                const Vec3f rotZ = rotation * Vec3f::PosZ;
                const float angle = -angleBetween(rotZ, onPlane, direction);
                const Mat4x4f matrix = translationMatrix(center) * rotationMatrix(direction, angle) * rotationMatrix(rotation) * translationMatrix(16.0f * Vec3f::PosX);
                
                for (size_t i = 0; i < 3; ++i)
                    vertices.push_back(Vertex(matrix * arrow[i]));
            }
            
            const size_t vertexCount = vertices.size();
            if (vertexCount == 0)
                return;
            
            VertexArray array = VertexArray::swap(GL_TRIANGLES, vertices);
            SetVboState vboState(m_vbo);
            vboState.mapped();
            array.prepare(m_vbo);
            vboState.active();
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);

            glDepthMask(GL_FALSE);

            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            shader.set("Color", m_angleColor);
            array.render();

            glPolygonMode(GL_FRONT, GL_FILL);
            glDepthMask(GL_TRUE);
        }

        Vec3f::List EntityRenderer::arrowHead(const float length, const float width) const {
            // clockwise winding
            Vec3f::List result(3);
            result[0] = Vec3f(0.0f,    width / 2.0f, 0.0f);
            result[1] = Vec3f(length,          0.0f, 0.0f);
            result[2] = Vec3f(0.0f,   -width / 2.0f, 0.0f);
            return result;
        }

        FontDescriptor EntityRenderer::font() {
            PreferenceManager& prefs = PreferenceManager::instance();
            const IO::Path& fontPath = prefs.get(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(prefs.get(Preferences::RendererFontSize));
            return FontDescriptor(fontPath, fontSize);
        }

        struct BuildColoredSolidBoundsVertices {
            VertexSpecs::P3NC4::Vertex::List& vertices;
            Color color;
            
            BuildColoredSolidBoundsVertices(VertexSpecs::P3NC4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4, const Vec3& n) {
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v1, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v2, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v3, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v3, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v4, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v1, n, color));
            }
        };

        struct BuildColoredWireframeBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredWireframeBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
            }
        };
        
        struct BuildWireframeBoundsVertices {
            VertexSpecs::P3::Vertex::List& vertices;
            
            BuildWireframeBoundsVertices(VertexSpecs::P3::Vertex::List& i_vertices) :
            vertices(i_vertices) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3::Vertex(v1));
                vertices.push_back(VertexSpecs::P3::Vertex(v2));
            }
        };
        
        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void EntityRenderer::validateBounds() {
            VertexSpecs::P3NC4::Vertex::List solidVertices;
            solidVertices.reserve(36 * m_entities.size());
            
            if (m_overrideBoundsColor) {
                VertexSpecs::P3::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                BuildWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices);
                Model::EntitySet::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_editorContext.visible(entity)) {
                        eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        if (!entity->hasChildren() && entity->model() == NULL) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(*entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        }
                    }
                }
                
                m_wireframeBoundsRenderer = EdgeRenderer(VertexArray::swap(GL_LINES, wireframeVertices));
            } else {
                VertexSpecs::P3C4::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                Model::EntitySet::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_editorContext.visible(entity)) {
                        if (!entity->hasChildren() && entity->model() == NULL) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(*entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        } else {
                            BuildColoredWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices, boundsColor(*entity));
                            eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        }
                    }
                }

                m_wireframeBoundsRenderer = EdgeRenderer(VertexArray::swap(GL_LINES, wireframeVertices));
            }
            
            m_solidBoundsRenderer = TriangleRenderer(VertexArray::swap(GL_TRIANGLES, solidVertices));
            m_boundsValid = true;
        }

        AttrString EntityRenderer::entityString(const Model::Entity* entity) const {
            const Model::AttributeValue& classname = entity->classname();
            const Model::AttributeValue& targetname = entity->attribute(Model::AttributeNames::Targetname);
            
            AttrString str;
            str.appendCentered(classname);
            if (!targetname.empty())
                str.appendCentered(targetname);
            return str;
        }

        const Color& EntityRenderer::boundsColor(const Model::Entity& entity) const {
            const Assets::EntityDefinition* definition = entity.definition();
            if (definition == NULL)
                return m_boundsColor;
            return definition->color();
        }
    }
}