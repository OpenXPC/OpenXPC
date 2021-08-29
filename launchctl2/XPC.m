#include <Foundation/NSObjCRuntime.h>
#import "XPC.h"

@implementation NSArray (XPC)

- (xpc_object_t)newXPCObject
{
	xpc_object_t array = xpc_array_create(NULL, 0);
	[self enumerateObjectsUsingBlock:^(id value, NSUInteger index,
		BOOL *stop) {
		xpc_object_t xpc = [value newXPCObject];
		xpc_array_set_value(array, XPC_ARRAY_APPEND, xpc);
		xpc_release(xpc);
	}];
	return array;
}
@end

@implementation NSData (XPCParse)

- (xpc_object_t)newXPCObject
{
	return xpc_data_create([self bytes], [self length]);
}

@end

@implementation NSDictionary (XPCParse)

- (xpc_object_t)newXPCObject
{
	xpc_object_t xpc = xpc_dictionary_create(NULL, NULL, 0);
	[self enumerateKeysAndObjectsUsingBlock:^(id<NSCopying> key, id value,
		BOOL *stop) {
		if ([value respondsToSelector:@selector(newXPCObject)]) {
			xpc_object_t valxpc = [value newXPCObject];
			xpc_dictionary_set_value(xpc,
				[(NSString *)key UTF8String], valxpc);
			xpc_release(valxpc);
		} else {
			NSLog(@"Failed to encode %@", value);
		}
	}];
	return xpc;
}

@end

@implementation NSFileHandle (XPCParse)

- (xpc_object_t)newXPCObject
{
	return xpc_fd_create([self fileDescriptor]);
}

@end

@implementation NSNumber (XPCParse)

- (xpc_object_t)newXPCObject
{
	const char *type = [self objCType];

	if (strcmp(type, @encode(BOOL)) == 0)
		return xpc_bool_create([self boolValue]);
	else if (strcmp(type, @encode(unsigned long)) == 0)
		return xpc_uint64_create([self unsignedLongValue]);
	else if (strcmp(type, @encode(long)) == 0)
		return xpc_int64_create([self longValue]);
	else if (strcmp(type, @encode(double)) == 0)
		return xpc_double_create([self doubleValue]);
	else
		NSLog(@"Failed to encode %@", self);

	return NULL;
}

@end

@implementation NSString (XPC)

- (xpc_object_t)newXPCObject
{
	return xpc_string_create([self UTF8String]);
}

@end
