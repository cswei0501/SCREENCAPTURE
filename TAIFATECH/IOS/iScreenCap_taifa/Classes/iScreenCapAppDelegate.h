//
//  iScreenCapAppDelegate.h
//  iScreenCap
//
//  Created by weiqiang xu on 11-6-24.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class iScreenCapViewController;

@interface iScreenCapAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    iScreenCapViewController *viewController;
}

- (BOOL) startRun;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet iScreenCapViewController *viewController;

@end

