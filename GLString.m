//
//  GLString.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLString.h"
#import "VBOMemBlock.h"
#import "GLFont.h"

@implementation GLString

- (id)initWithMemBlock:(VBOMemBlock *)theMemBlock glFont:(GLFont *)theFont {
    if (theMemBlock == nil)
        [NSException raise:NSInvalidArgumentException format:@"mem block must not be nil"];
    if (theFont == nil)
        [NSException raise:NSInvalidArgumentException format:@"font must not be nil"];
    
    if (self = [self init]) {
        memBlock = [theMemBlock retain];
        glFont = [theFont retain];
    }
    
    return self;
}

- (void)render {
    [glFont activate];
    [memBlock activate];
    glInterleavedArrays(GL_T2F_V3F, 0, NULL);
    glDrawArrays(GL_QUADS, [memBlock address] / (5 * sizeof(float)), [memBlock capacity] / (5 * sizeof(float)));
    [memBlock deactivate];
    [glFont deactivate];
}

- (void)dealloc {
    [memBlock free];
    [memBlock release];
    [glFont release];
    [super dealloc];
}
@end
