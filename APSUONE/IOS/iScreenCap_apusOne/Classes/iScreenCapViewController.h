//
//  iScreenCapViewController.h
//  iScreenCap
//
//  Created by weiqiang xu on 11-6-24.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface iScreenCapViewController : UIViewController {
	IBOutlet UISwitch* mSwitch;
	IBOutlet UIProgressView* mProgressView;
}

- (IBAction) SwitchUpdate;
@end

