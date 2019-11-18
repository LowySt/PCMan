//
//  AppDelegate.m
//  HyperPCMan
//
//  Created by Lowy on 13/11/2019.
//  Copyright © 2019 Lowy. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)SetupInitTab:(NSTabViewItem *)item {
    
    NSArray *HeroNames = @[ @"Gremag", @"Federico", @"Ken Shiro", @"Sirion", @"Albion", @"Sdentato", @"Dresdam", @"Zoddak"];
           
    NSArray *AllyNames = @[@"Ally 1", @"Ally 2", @"Ally 3", @"Ally 4"];
    
    NSArray *EnemyNames = @[ @"Enemy 1", @"Enemy 2", @"Enemy 3", @"Enemy 4", @"Enemy 5", @"Enemy 6", @"Enemy 7", @"Enemy 8", @"Enemy 9", @"Enemy 10", @"Enemy 11", @"Enemy 12", @"Enemy 13", @"Enemy 14", @"Enemy 15", @"Enemy 16"];
    
    int yPos = 750;
    for(int i = 0; i < mainViewController->PARTY_SIZE; i++) {
    
        LabeledTextBox *Hero = [[LabeledTextBox alloc]
                            initLabeled:HeroNames[i] labelDir:LABEL_LEFT
                            p:CGPointMake(525, yPos) isEditable:true];
        [Hero->Box setFrameSize:NSMakeSize(30, 20)];
    
        [[item view] addSubview:Hero->Box];
        [[item view] addSubview:Hero->Label];
        mainViewController->Heros[i] = Hero;
        yPos -= 30;
    }
        
    for(int i = 0; i < mainViewController->ALLY_SIZE; i++) {
    
        LabeledTextBox *Ally = [[LabeledTextBox alloc]
                            initEditableLabel:AllyNames[i] labelDir:LABEL_LEFT
                            frame:NSMakeRect(525, yPos, 70, 20) isEditable:true
                            boxSize:NSMakeSize(30, 20)];
           
        [[item view] addSubview:Ally->Box];
    	[[item view] addSubview:Ally->Label];
        mainViewController->Allies[i] = Ally;
        yPos -= 30;
    }
 
    yPos = 750;
    for(int i = 0; i < mainViewController->MOB_SIZE; i++) {
    
        LabeledTextBox *Mob = [[LabeledTextBox alloc]
                            initEditableLabel:EnemyNames[i] labelDir:LABEL_LEFT
                           frame:NSMakeRect(365, yPos, 70, 20) isEditable:true
                           boxSize:NSMakeSize(30, 20)];
        
        [[item view] addSubview:Mob->Box];
        [[item view] addSubview:Mob->Label];
        mainViewController->Mobs[i] = Mob;
        yPos -= 30;
    }
    
    yPos = 750;
    for(int i = 0; i < mainViewController->ORDER_SIZE; i++) {
        OrderField *Order = [[OrderField alloc]
                             initOrder:CGPointMake(680, yPos) num:i+1];
        
        [[item view] addSubview:Order->Name];
        [[item view] addSubview:Order->Num];
        mainViewController->Order[i] = Order;
        yPos -= 22;
    }
    
    /*setField(_InitiativeRollButton, CGPointMake(420, 836));
    setField(_ResetButton, CGPointMake(20, 870));
    setField(_MapButton, CGPointMake(100, 870));
    setField(_Counter1Button, CGPointMake(160, 803));
    setField(_Counter2Button, CGPointMake(160, 763));
    setField(_Counter3Button, CGPointMake(160, 723));
    setField(_Counter4Button, CGPointMake(160, 683));**/
    
    ActionButton *Reset = [[ActionButton alloc] initWithAction:NSMakeRect(20, 810, 80, 20) name:@"Reset" blk:^void(){
        NSLog(@"Reset Button!");
    }];
        
    ActionButton *Map = [[ActionButton alloc] initWithAction:NSMakeRect(100, 810, 80, 20) name:@"Map" blk:^void(){
        NSLog(@"Map Button!");
    }];
        
    ActionButton *Roll = [[ActionButton alloc] initWithAction:NSMakeRect(420, 780, 80, 20) name:@"Roll" blk:^void(){
        NSLog(@"Roll Button!");
    }];
    
    ActionButton *Set = [[ActionButton alloc] initWithAction:NSMakeRect(730, 780, 80, 20) name:@"Set" blk:^void(){
        NSLog(@"Set Button!");
    }];
    
    ActionButton *Next = [[ActionButton alloc] initWithAction:NSMakeRect(900, 780, 80, 20) name:@"Next" blk:^void(){
        NSLog(@"Next Button!");
    }];
    
    [[item view] addSubview:Reset->Button];
    [[item view] addSubview:Map->Button];
    [[item view] addSubview:Roll->Button];
    [[item view] addSubview:Set->Button];
    [[item view] addSubview:Next->Button];
    mainViewController->Reset = Reset;
    mainViewController->Map = Map;
    mainViewController->Roll = Roll;
    mainViewController->Set = Set;
    mainViewController->Next = Next;

}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    mainViewController = (ViewController *) NSApplication.sharedApplication.orderedWindows.firstObject.contentViewController;
    
    mainViewController->PARTY_SIZE = 8;
    mainViewController->ALLY_SIZE  = 4;
    mainViewController->MOB_SIZE   = 16;
    mainViewController->ORDER_SIZE = 28;
    
    NSWindow *MainWindow = [NSApp windows][0];
    [MainWindow setFrame:NSMakeRect(300, 160, 1280, 960) display:true];
    
    NSTabView __strong *MainTabView = [[NSTabView alloc] initWithFrame:NSMakeRect(0, 0, 1280, 890)];
    NSTabViewItem *item0 = [[NSTabViewItem alloc] init];
    [item0 setLabel:@"PC"];
    NSTabViewItem *item1 = [[NSTabViewItem alloc] init];
    [item1 setLabel:@"Feats"];
    NSTabViewItem *item2 = [[NSTabViewItem alloc] init];
    [item2 setLabel:@"Init"];
    NSTabViewItem *item3 = [[NSTabViewItem alloc] init];
    [item3 setLabel:@"Party"];

    [self SetupInitTab:item2];
    
    NSArray *tabViewItems = [[NSArray alloc] initWithObjects:item0, item1, item2, item3, nil];
    [MainTabView setTabViewItems:tabViewItems];
    [MainTabView selectTabViewItemAtIndex:2];
    [[MainWindow contentView] addSubview:MainTabView];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end