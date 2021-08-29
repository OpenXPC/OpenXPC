/*
 *		PROPRIETARY NOTICE
 *
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from LaunchD-Reloaded(tm).
 *
 *
 *		Copyright Notice
 *
 *  Notice of copyright on this source code product does not indicate
 *  publication.
 *
 *	(c) 2021 The Project Maintainers of LaunchD-Reloaded.
 *		All rights reserved.
 */

#ifndef XPC_H_
#define XPC_H_

#include <Foundation/Foundation.h>

#include "xpc2/xpc.h"

@interface NSArray (XPC)
- (xpc_object_t)newXPCObject;
@end

@interface NSData (XPC)
- (xpc_object_t)newXPCObject;
@end

@interface NSDictionary (XPC)
- (xpc_object_t)newXPCObject;
@end

@interface NSFileHandle (XPC)
- (xpc_object_t)newXPCObject;
@end

@interface NSNumber (XPC)
- (xpc_object_t)newXPCObject;
@end

@interface NSString (XPC)
- (xpc_object_t)newXPCObject;
@end

#endif /* XPC_H_ */
