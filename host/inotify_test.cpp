#include <stdio.h>
#include "inotify.h"
#include <sys/poll.h>

struct inotify_event_with_path {
	__s32		wd;		/* watch descriptor */
	__u32		mask;		/* watch mask */
	__u32		cookie;		/* cookie to synchronize two events */
	__u32		len;		/* length (including nulls) of name */
	char		name[256];	/* stub for possible name */
};

int main( int argc, char const *const argv[] ){
	printf( "initializing inotify\n" );
	int infd = inotify_init();
	if( 0 <= infd ){
		printf( "inotify_init success: %d\n", infd );
		__u32 evmask = // IN_CREATE | IN_DELETE ;
			       0xEFFF & ~IN_MODIFY & ~IN_ACCESS ;
		printf( "mask == 0x%x\n", evmask );
		for( int arg = 1 ; arg < argc ; arg++ ){
			int rv = inotify_add_watch(infd,argv[arg],evmask);
			printf( "\t%d\t%s\n", infd, argv[arg]);
		}

		while( 1 ){
			pollfd pfd ;
			pfd.fd = infd ;
			pfd.events = POLLIN ;
			int numReady = poll(&pfd,1,1000);
			if( 0 < numReady ){
				struct inotify_event_with_path ev ;
				int numRead = read(infd,&ev,sizeof(ev));
				if( numRead >= sizeof(struct inotify_event)){
					ev.name[ev.len] = 0 ;
					printf( "%d\t%x\t%x\t%u\t%s\n", ev.wd,ev.mask,ev.cookie,ev.len,ev.name );
				}
				else {
					printf( "short read %d/%d\n", numRead,sizeof(struct inotify_event));
					break ;
				}
			}
			else
				printf( "idle\n" );
		}
	}
	else
		perror( "inotify_init" );

	return 0 ;
}
