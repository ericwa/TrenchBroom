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

#ifndef __TrenchBroom__PointHandle__
#define __TrenchBroom__PointHandle__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class PointHandle {
        private:
            Vec3 m_position;
            Color m_color;
        public:
            PointHandle(const Vec3& position, const Color& color);
            
            FloatType pick(const Ray3& pickRay, const Renderer::Camera& camera) const;
            void render(const Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, bool highlight) const;
        };
    }
}

#endif /* defined(__TrenchBroom__PointHandle__) */