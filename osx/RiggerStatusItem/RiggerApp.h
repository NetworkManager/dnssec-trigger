//
//  RiggerApp.h
//  RiggerStatusItem
//
//  Created by Wouter Wijngaards on 8/29/11.
//  Copyright 2011 NLnet Labs. All rights reserved.
//

#import <Cocoa/Cocoa.h>
struct cfg;

@interface RiggerApp : NSObject {
	/* outlets connect to the interface */
	IBOutlet NSMenu* riggermenu;
	NSStatusItem* riggeritem;
	NSImage* icon;
	NSImage* icon_alert;
	IBOutlet NSWindow* resultwindow;
	IBOutlet NSTextView* resultpane;
	IBOutlet NSWindow* unsafewindow;
	IBOutlet NSTextField* unsafepane;
	
	/** if we have asked about disconnect or insecure */
	int unsafe_asked;
	/** configuration */
	struct cfg* cfg;
}

/* IBAction to connect to the routine that takes actions after menu */
-(IBAction)Reprobe:(id)sender;
-(IBAction)ProbeResults:(id)sender;
-(IBAction)ProbeResultsOK:(id)sender;
-(IBAction)UnsafeInsecure:(id)sender;
-(IBAction)UnsafeDisconnect:(id)sender;
-(BOOL)windowShouldClose:(NSWindow*)sender;
-(void)SpawnFeed:(id)param;
-(void)PanelAlert;
-(void)PresentUnsafeDialog;
-(void)PanelAlertDanger;
-(void)PanelAlertSafe;

@end