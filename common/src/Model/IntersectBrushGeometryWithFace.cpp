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

#include "IntersectBrushGeometryWithFace.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        IntersectBrushGeometryWithFace::IntersectBrushGeometryWithFace(BrushGeometry& geometry, BrushFace* face) :
        BrushAlgorithm(geometry),
        m_face(face) {
            assert(m_face != NULL);
            m_remainingVertices.reserve(24);
            m_droppedVertices.reserve(24);
            m_remainingEdges.reserve(32);
            m_droppedEdges.reserve(32);
        }
        
        const BrushVertexList& IntersectBrushGeometryWithFace::vertices() const {
            return m_remainingVertices;
        }
        
        const BrushEdgeList& IntersectBrushGeometryWithFace::edges() const {
            return m_remainingEdges;
        }
        
        const BrushFaceGeometryList& IntersectBrushGeometryWithFace::sides() const {
            return m_remainingSides;
        }

        AddFaceResult::Code IntersectBrushGeometryWithFace::doExecute(BrushGeometry& geometry) {
            if (isFaceIdenticalWithAnySide(geometry))
                return AddFaceResult::Code_FaceRedundant;
            
            const AddFaceResult::Code processVerticesResult = processVertices(geometry);
            if (processVerticesResult != AddFaceResult::Code_BrushSplit)
                return processVerticesResult;
            processEdges(geometry);
            processSides(geometry);
            
            createNewSide();
            cleanup();
            
            return AddFaceResult::Code_BrushSplit;
        }
        
        bool IntersectBrushGeometryWithFace::isFaceIdenticalWithAnySide(BrushGeometry& geometry) {
            BrushFaceGeometryList::const_iterator it, end;
            for (it = geometry.sides.begin(), end = geometry.sides.end(); it != end; ++it) {
                BrushFaceGeometry* side = *it;
                if (side->face != NULL) {
                    if (m_face->arePointsOnPlane(side->face->boundary()))
                        return true;
                }
            }
            return false;
        }
        
        AddFaceResult::Code IntersectBrushGeometryWithFace::processVertices(BrushGeometry& geometry) {
            const Plane3& boundary = m_face->boundary();

            size_t drop = 0;
            size_t keep = 0;
            size_t undecided = 0;
            
            BrushVertexList::const_iterator it, end;
            for (it = geometry.vertices.begin(), end = geometry.vertices.end(); it != end; ++it) {
                BrushVertex* vertex = *it;
                vertex->updateMark(boundary);
                
                switch (vertex->mark) {
                    case BrushVertex::Mark_Drop:
                        drop++;
                        m_droppedVertices.push_back(vertex);
                        break;
                    case BrushVertex::Mark_Keep:
                        keep++;
                        m_remainingVertices.push_back(vertex);
                        break;
                    case BrushVertex::Mark_Undecided:
                        undecided++;
                        m_remainingVertices.push_back(vertex);
                        break;
                    case BrushVertex::Mark_New:
                        assert(false);
                        break;
                }
            }
            
            assert(drop + keep + undecided == geometry.vertices.size());
            if (drop + undecided == geometry.vertices.size())
                return AddFaceResult::Code_BrushNull;
            if (keep + undecided == geometry.vertices.size())
                return AddFaceResult::Code_FaceRedundant;
            return AddFaceResult::Code_BrushSplit;
        }

        void IntersectBrushGeometryWithFace::processEdges(BrushGeometry& geometry) {
            const Plane3& boundary = m_face->boundary();
            
            BrushEdgeList::const_iterator it, end;
            for (it = geometry.edges.begin(), end = geometry.edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                edge->updateMark();
                
                switch (edge->mark) {
                    case BrushEdge::Mark_Drop: {
                        m_droppedEdges.push_back(edge);
                        break;
                    }
                    case BrushEdge::Mark_Keep: {
                        m_remainingEdges.push_back(edge);
                        break;
                    }
                    case BrushEdge::Mark_Undecided: {
                        m_remainingEdges.push_back(edge);
                        break;
                    }
                    case BrushEdge::Mark_Split: {
                        BrushVertex* newVertex = edge->split(boundary);
                        assert(newVertex != NULL);
                        m_remainingVertices.push_back(newVertex);
                        m_remainingEdges.push_back(edge);
                        break;
                    }
                    case BrushEdge::Mark_New:
                        assert(false);
                        break;
                }
            }
        }

        void IntersectBrushGeometryWithFace::processSides(BrushGeometry& geometry) {
            BrushFaceGeometryList::const_iterator it, end;
            for (it = geometry.sides.begin(), end = geometry.sides.end(); it != end; ++it) {
                BrushFaceGeometry* side = *it;
                switch (side->mark()) {
                    case BrushFaceGeometry::Mark_Drop: {
                        m_droppedSides.push_back(side);
                        if (side->face != NULL)
                            removeFace(side->face);
                        break;
                    }
                    case BrushFaceGeometry::Mark_Keep: {
                        BrushEdge* undecidedEdge = side->findUndecidedEdge();
                        if (undecidedEdge != NULL) {
                            if (undecidedEdge->right == side)
                                undecidedEdge->flip();
                            undecidedEdge->setRightNull();
                            m_newSideEdges.push_back(undecidedEdge);
                        }
                        m_remainingSides.push_back(side);
                        break;
                    }
                    case BrushFaceGeometry::Mark_Split: {
                        BrushEdge* newEdge = side->splitUsingEdgeMarks();
                        assert(newEdge != NULL);
                        assert(!side->containsDroppedEdge());
                        
                        m_newSideEdges.push_back(newEdge);
                        m_remainingEdges.push_back(newEdge);
                        m_remainingSides.push_back(side);
                        break;
                    }
                }
            }
        }
        
        void IntersectBrushGeometryWithFace::createNewSide() {
            BrushFaceGeometry* newSide = new BrushFaceGeometry();

            BrushEdgeList::iterator it1, it2, end;
            for (it1 = m_newSideEdges.begin(), end = m_newSideEdges.end(); it1 != end; ++it1) {
                BrushEdge* edge = *it1;
                if (std::distance(it1, end) > 2) {
                    for (it2 = it1 + 2; it2 < end; ++it2) {
                        const BrushEdge* candidate = *it2;
                        if (edge->end == candidate->start)
                            std::iter_swap(it1 + 1, it2);
                    }
                }
                
                newSide->addForwardEdge(edge);
            }

            assert(newSide->isClosed());
            
            newSide->face = m_face;
            m_face->setSide(newSide);
            addFace(m_face);
            m_remainingSides.push_back(newSide);
        }
        
        void IntersectBrushGeometryWithFace::cleanup() {
            assert(checkDroppedEdges());
            VectorUtils::clearAndDelete(m_droppedSides);
            VectorUtils::clearAndDelete(m_droppedEdges);
            VectorUtils::clearAndDelete(m_droppedVertices);
        }

        bool IntersectBrushGeometryWithFace::checkDroppedEdges() {
            BrushEdgeList::const_iterator eIt, eEnd;
            for (eIt = m_droppedEdges.begin(), eEnd = m_droppedEdges.end(); eIt != eEnd; ++eIt) {
                BrushEdge* edge = *eIt;

                BrushFaceGeometryList::const_iterator sIt, sEnd;
                for (sIt = m_remainingSides.begin(), sEnd = m_remainingSides.end(); sIt != sEnd; ++sIt) {
                    BrushFaceGeometry* side = *sIt;
                    if (std::find(side->edges.begin(), side->edges.end(), edge) != side->edges.end())
                        return false;
                }
            }
            return true;
        }
    }
}