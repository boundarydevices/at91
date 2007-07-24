#ifndef __INOTIFY_H__
#define __INOTIFY_H__ "$Id$"

/*
 * inotify.h
 *
 * This header file declares the inotify_x system calls
 * for GLIBC versions not compiled against kernel headers
 * for version 2.6.13+
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "linux/inotify.h"
#include "asm/unistd.h"
#include <unistd.h>

inline int inotify_init (void)
{
	return syscall (__NR_inotify_init);
}

inline int inotify_add_watch (int fd, const char *name, __u32 mask)
{
	return syscall (__NR_inotify_add_watch, fd, name, mask);
}

inline int inotify_rm_watch (int fd, __u32 wd)
{
	return syscall (__NR_inotify_rm_watch, fd, wd);
}

#endif

