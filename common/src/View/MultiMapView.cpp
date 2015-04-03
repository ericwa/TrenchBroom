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

#include "MultiMapView.h"

#include "View/MapView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MultiMapView::MultiMapView(wxWindow* parent) :
        MapViewContainer(parent) {}
        
        MultiMapView::~MultiMapView() {}

        void MultiMapView::addMapView(MapView* mapView) {
            assert(mapView != NULL);
            m_mapViews.push_back(mapView);
        }

        void MultiMapView::doFlashSelection() {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->flashSelection();
            }
        }

        bool MultiMapView::doGetIsCurrent() const {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                if (mapView->isCurrent())
                    return true;
            }
            return false;
        }

        void MultiMapView::doSetToolBoxDropTarget() {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->setToolBoxDropTarget();
            }
        }
        
        void MultiMapView::doClearDropTarget() {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->clearDropTarget();
            }
        }

        Vec3 MultiMapView::doGetPasteObjectsDelta(const BBox3& bounds) const {
            MapView* current = currentMapView();
            assert(current != NULL);
            return current->pasteObjectsDelta(bounds);
        }
        
        void MultiMapView::doCenterCameraOnSelection() {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->centerCameraOnSelection();
            }
        }
        
        void MultiMapView::doMoveCameraToPosition(const Vec3& position) {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->moveCameraToPosition(position);
            }
        }
        
        void MultiMapView::doMoveCameraToCurrentTracePoint() {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                mapView->moveCameraToCurrentTracePoint();
            }
        }

        MapView* MultiMapView::currentMapView() const {
            MapViewList::const_iterator it, end;
            for (it = m_mapViews.begin(), end = m_mapViews.end(); it != end; ++it) {
                MapView* mapView = *it;
                if (mapView->isCurrent())
                    return mapView;
            }
            return NULL;
        }
    }
}