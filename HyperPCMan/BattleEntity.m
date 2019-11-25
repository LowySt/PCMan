//
//  BattleEntity.m
//  HyperPCMan
//
//  Created by Lowy on 20/11/2019.
//  Copyright © 2019 Lowy. All rights reserved.
//

#import "BattleEntity.h"

@implementation BattleEntity

- (BattleEntity *)initWithFrame:(NSRect)frame name:(NSString *)name {

    NSNumberFormatter *nf = [[NSNumberFormatter alloc] init];
    [nf setAllowsFloats:false];
    [nf setNumberStyle:NSNumberFormatterNoStyle];
    [nf setPartialStringValidationEnabled:true];

    Box = [[LabeledTextBox alloc] initEditableLabel:name labelDir:LABEL_LEFT frame:frame isEditable:true boxSize:NSMakeSize(30, 20)];
    Box->Box.formatter = nf;
    [Box->Box setAlignment:NSTextAlignmentCenter];
    [Box->Box setStringValue:@"0"];
        
    NSRect initFrame = NSMakeRect(frame.origin.x + 30, frame.origin.y, 30, 20);
    Init = [[NSTextField alloc] initWithFrame:initFrame];
    Init.formatter = nf;
    [Init setAlignment:NSTextAlignmentCenter];
    
    [Box->Box setHidden:true];
    [Box->Label setHidden:true];
    [Init setHidden:true];
    
    return self;
}

- (void)hide {
    [Box->Box setHidden:true];
    [Box->Label setHidden:true];
    [Init setHidden:true];
}

- (void)show {
    [Box->Box setHidden:false];
    [Box->Label setHidden:false];
    [Init setHidden:false];
}

//NOTE: I hate ""OOP"" languages...
- (void)setName:(NSString *)name {
    [Box->Label setStringValue:name];
}


@end
