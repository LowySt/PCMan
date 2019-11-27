//
//  StatField.h
//  HyperPCMan
//
//  Created by Lowy on 27/11/2019.
//  Copyright © 2019 Lowy. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LabeledTextBox.h"
NS_ASSUME_NONNULL_BEGIN

#define STATS_NUM 6

static NSString * _Nonnull __strong Stat[STATS_NUM] = {@"STR", @"DEX", @"CON", @"INT", @"WIS", @"CHA"};

@interface StatField : NSObject {
    
    @public LabeledTextBox __strong *Box;
    @public NSTextField __strong *Bonus;
};
    
- (StatField *)initWithLabel:(NSString *)label frame:(NSRect)frame;

@end

NS_ASSUME_NONNULL_END
