#ifndef FBSBUF_H_
#define FBSBUF_H_

#include "xpc2/config.h"

#ifndef HAVE_sys_sbuf_h
#include "sbuf.h"
#else
#include <sys/sbuf.h>
#endif

#endif /* FBSBUF_H_ */
