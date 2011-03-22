//
//  SelectionLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"

@class LineRenderer;
@class Options;

@interface SelectionLayer : GeometryLayer {
    LineRenderer* gridRenderer;
    Options* options;
    NSMutableDictionary* gridFigures;
}

@end
