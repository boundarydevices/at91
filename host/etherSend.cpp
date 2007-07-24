/*
 * etherSend.cpp
 *
 * Transmit an ethernet packet on the attached AT91SAM7x
 * (for test purposes).
 *
 */
#include <stdio.h>
#include "msgEthernet.h"
#include "appMsgDirect.h"
#include "sambaUtil.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "hexDump.h"

#define ETH_ALEN        6
struct ethhdr
{
        unsigned char   h_dest[ETH_ALEN];
        unsigned char   h_source[ETH_ALEN];
        unsigned short  h_proto;
} __attribute__((packed));

#define networkLong( __hihi, __hilo, __lohi, __lolo ) (0x##__lolo##__lohi##__hilo##__hihi##UL)
#define networkWord( __hi, __lo )      ((unsigned short)( 0x##__lo##__hi ))

static unsigned short const protoArp = networkWord( 08, 06 );
static unsigned short const protoIp  = networkWord( 08, 00 );
static unsigned char const ipProtoICMP = '\x01' ;
static unsigned short const arpRequest = networkWord( 00, 01 );
static unsigned short const arpResponse = networkWord( 00, 02 );
static unsigned short const arpEther = networkWord( 00, 01 );
static unsigned short const arpIp = networkWord( 08, 00 );
static unsigned long const defaultNetMask_ = networkLong( ff, ff, ff, 00 );

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet/TR/TB	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
#define ARPHRD_DLCI	15		/* Frame Relay DLCI		*/
#define ARPHRD_ATM	19		/* ATM 				*/
#define ARPHRD_METRICOM	23		/* Metricom STRIP (new IANA id)	*/
#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/
#define ARPHRD_EUI64	27		/* EUI-64                       */

/* Dummy types for non ARP hardware */
#define ARPHRD_SLIP	256
#define ARPHRD_CSLIP	257
#define ARPHRD_SLIP6	258
#define ARPHRD_CSLIP6	259
#define ARPHRD_RSRVD	260		/* Notional KISS type 		*/
#define ARPHRD_ADAPT	264
#define ARPHRD_ROSE	270
#define ARPHRD_X25	271		/* CCITT X.25			*/
#define ARPHRD_HWX25	272		/* Boards with X.25 in firmware	*/
#define ARPHRD_PPP	512
#define ARPHRD_CISCO	513		/* Cisco HDLC	 		*/
#define ARPHRD_HDLC	ARPHRD_CISCO
#define ARPHRD_LAPB	516		/* LAPB				*/
#define ARPHRD_DDCMP    517		/* Digital's DDCMP protocol     */
#define ARPHRD_RAWHDLC	518		/* Raw HDLC			*/

#define ARPHRD_TUNNEL	768		/* IPIP tunnel			*/
#define ARPHRD_TUNNEL6	769		/* IPIP6 tunnel			*/
#define ARPHRD_FRAD	770             /* Frame Relay Access Device    */
#define ARPHRD_SKIP	771		/* SKIP vif			*/
#define ARPHRD_LOOPBACK	772		/* Loopback device		*/
#define ARPHRD_LOCALTLK 773		/* Localtalk device		*/
#define ARPHRD_FDDI	774		/* Fiber Distributed Data Interface */
#define ARPHRD_BIF      775             /* AP1000 BIF                   */
#define ARPHRD_SIT	776		/* sit0 device - IPv6-in-IPv4	*/
#define ARPHRD_IPDDP	777		/* IP over DDP tunneller	*/
#define ARPHRD_IPGRE	778		/* GRE over IP			*/
#define ARPHRD_PIMREG	779		/* PIMSM register interface	*/
#define ARPHRD_HIPPI	780		/* High Performance Parallel Interface */
#define ARPHRD_ASH	781		/* Nexus 64Mbps Ash		*/
#define ARPHRD_ECONET	782		/* Acorn Econet			*/
#define ARPHRD_IRDA 	783		/* Linux-IrDA			*/
/* ARP works differently on different FC media .. so  */
#define ARPHRD_FCPP	784		/* Point to point fibrechannel	*/
#define ARPHRD_FCAL	785		/* Fibrechannel arbitrated loop */
#define ARPHRD_FCPL	786		/* Fibrechannel public loop	*/
#define ARPHRD_FCFABRIC	787		/* Fibrechannel fabric		*/
	/* 787->799 reserved for fibrechannel media types */
#define ARPHRD_IEEE802_TR 800		/* Magic type ident for TR	*/
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#define ARPHRD_IEEE80211_PRISM 802	/* IEEE 802.11 + Prism2 header  */

#define ARPHRD_VOID	  0xFFFF	/* Void type, nothing is known */

/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/
#define	ARPOP_InREQUEST	8		/* InARP request		*/
#define	ARPOP_InREPLY	9		/* InARP reply			*/
#define	ARPOP_NAK	10		/* (ATM)ARP NAK			*/


/* ARP Flag values. */
#define ATF_COM		0x02		/* completed entry (ha valid)	*/
#define	ATF_PERM	0x04		/* permanent entry		*/
#define	ATF_PUBL	0x08		/* publish entry		*/
#define	ATF_USETRAILERS	0x10		/* has requested trailers	*/
#define ATF_NETMASK     0x20            /* want to use a netmask (only
					   for proxy entries) */
#define ATF_DONTPUB	0x40		/* don't answer this addresses	*/

struct arphdr
{
	unsigned short	ar_hrd;		/* format of hardware address	*/
	unsigned short	ar_pro;		/* format of protocol address	*/
	unsigned char	ar_hln;		/* length of hardware address	*/
	unsigned char	ar_pln;		/* length of protocol address	*/
	unsigned short	ar_op;		/* ARP opcode (command)		*/

	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/
};

/*
A000DA3E   FF FF FF FF FF FF
           00 03 47 2E B4 78
           08 06                 arp
           00 01                 ARPHRD_ETHER
A000DA4E   08 00                 ETH_P_IP
           06                    ETH_ALEN
           04                    sizeof( ip_ )
           00 01                 ARPOP_REQUEST
           00 03 47 2E B4 78     requester mac
           C0 A8 00 9E           requester ip
A000DA5E   00 00 00 00 00 00     target mac
           C0 A8 00 6F           target ip
           00 00 00 00 00 00
A000DA6E   00 00 00 00 00 00
           00 00 00 00 00 00               ............
*/
static unsigned char const __arp_packet[] = {
   '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'       // dest mac
,  '\x00', '\xA0', '\xDE', '\xAD', '\xBE', '\xEF'       // src mac
,  '\x08', '\x06'                                       // arp
,  '\x00', '\x01'                                       // ARPHRD_ETHER
,  '\x08', '\x00'                                       // ETH_P_IP
,  '\x06'                                               // ETH_ALEN
,  '\x04'                                               // sizeof( ip_ )
,  '\x00', '\x01'                                       // ARPOP_REQUEST
,  '\x00', '\xA0', '\xDE', '\xAD', '\xBE', '\xEF'       // requester mac
,  '\xC0', '\xA8', '\x01', '\x33'                       // requester ip
,  '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'       // target mac
,  '\xC0', '\xA8', '\x01', '\x19'                       // target ip
,  '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
,  '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08'
,  '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18'
};

struct arp_packet_t {
   struct ethhdr eth ;
   struct arphdr arp ;
} __attribute__((packed));

struct arp_packet_t arp_packet = {
   eth: {
      h_dest:   { '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF' }
,     h_source: { '\x00', '\xA0', '\xDE', '\xAD', '\xBE', '\xEF' }
,     h_proto:  protoArp
   }
,  arp: {
	ar_hrd: arpEther		/* format of hardware address	*/
,	ar_pro: protoIp 		/* format of protocol address	*/
,	ar_hln: ETH_ALEN		/* length of hardware address	*/
,	ar_pln: 4	    		/* length of protocol address	*/
,	ar_op: arpRequest		/* ARP opcode (command)		*/
   }
};

struct request_t {
	struct appHeader_t 	appHdr_ ;
        unsigned short          msgType_ ;
	arp_packet_t            packet_ ;
};

static char const * const devName = "/dev/at91sam7x0" ;
static unsigned char targetIp[4] = { 
   '\xC0', '\xA8', '\x01', '\x19'
};

static int fromDotted( char const *ips, unsigned char *ipb ){
   unsigned octets = 0 ;
   unsigned octet = 0 ;
   do {
      char const c = *ips++ ;
      if( ('0' <= c) && ('9' >= c) ){
         octet *= 10 ;
         octet += (c-'0');
      } else if( '.' == c ) {
         if( 3 > octets ){
            *ipb++ = octet ;
            octet = 0 ;
            octets++ ;
         }
         else {
            printf( "Too many octets\n" );
            return 0 ;
         }
      } else if( '\0' == c ){
         *ipb++ = octet ;
         octets++ ;
         if( 4 > octets ){
            return 0 ;
         }
      } else {
            printf( "Invalid digit <%c>\n", c );
            return 0 ;
      }
   } while( 4 > octets );

   return 1 ;
}

int main( int argc, char const * const argv[] )
{
	printf( "Hello, %s\n", argv[0] );
	
        if( 2 <= argc ){
           if( !fromDotted( argv[1], targetIp ) ){
              printf( "Invalid ip <%s>\n", argv[1] );
              return -1 ;
           }
        }

        printf( "Arp to target ip: %u.%u.%u.%u\n", targetIp[0],targetIp[1],targetIp[2],targetIp[3] );

	int fdDev = open( devName, O_RDWR );
	
	if( 0 <= fdDev ){
		char const *imageName ;
		unsigned length, crc ;
		if( samba_verify( fdUpload, imageName, length, crc ) ){
			struct request_t req ;
			setAppHeader( req.appHdr_, USBAPP_ETHERNET, sizeof(req)-sizeof(req.appHdr_) );
			req.msgType_ = MSGETHERNET_TRANSMIT ;
                        memcpy( &req.packet_, &arp_packet, sizeof( arp_packet ) );

	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
                        memcpy( req.packet_.arp.ar_sha, __arp_packet+6, sizeof( req.packet_.arp.ar_sha ) );
                        memset( req.packet_.arp.ar_sip, 0, sizeof(req.packet_.arp.ar_sip) );
                        memcpy( req.packet_.arp.ar_tha, __arp_packet, sizeof(req.packet_.arp.ar_tha) ); /* target hardware address	*/
                        memcpy( req.packet_.arp.ar_tip, targetIp, sizeof(req.packet_.arp.ar_tip)); /* target IP address		*/
                        unsigned requestSize = sizeof(req.appHdr_)
                                             + sizeof(req.msgType_)
                                             + sizeof(req.packet_);
			int numWritten = write( fdDev, &req, requestSize );
			printf( "wrote %d of %u bytes\n", numWritten, sizeof(req) );
                        printf( "appHdr 0x%x\n"
                                "ethHdr 0x%x\n"
                                "packet 0x%x\n"
                                , sizeof(req.appHdr_)
                                , sizeof(req.msgType_)
                                , sizeof(req.packet_) );
                        while( 1 ){
                           unsigned char responseBuf[2048];
                           int numRead = read( fdDev, responseBuf, sizeof( responseBuf ) );
                           if( 0 < numRead ){
                              if( sizeof(appHeader_t) < numRead ){
                                 appHeader_t const &hdr = *(appHeader_t const *)responseBuf ;
                                 if( !validAppHeader( hdr, 1544 ) ){
                                    printf( "--- not for us\r\n" );
                                    continue ;
                                 }

                                 if( USBAPP_ETHERNET != hdr.appId ){
                                    printf( "--- not ethernet %u\n", hdr.appId );
                                    continue ;
                                 }

                                 if( numRead < hdr.length + sizeof(appHeader_t) ){
                                    unsigned left = hdr.length + sizeof(appHeader_t) - numRead ;
                                    printf( "read %u bytes more\n", left );
                                    int more = read( fdDev, responseBuf+numRead, left );
                                    if( more != left ){
                                       printf( "short read %d of %u\n", more, left );
                                       continue ;
                                    }
                                    numRead += more ;
                                 }

                                 msgEthernetResponse_t const &ersp = *(msgEthernetResponse_t *)(&hdr+1);
                                 if( MSGETHERNET_RECEIVE != ersp.msgType_ ){
                                    printf( "--- not ethernet rx: %u\n", ersp.msgType_ );
                                    continue ;
                                 }
                                 if( 0 != ersp.result_ ){
                                    printf( "--- bad rx status: %u\n", ersp.result_ );
                                    continue ;
                                 }
                                 unsigned char const *data = (unsigned char const *)( &ersp+1 );
                                 printf( "--- first byte of response == %02x\n", *data );

                                 unsigned payload = numRead-(data-responseBuf);
                                 if( payload >= sizeof(struct arp_packet_t) ){
                                    struct arp_packet_t const &rsp = *(struct arp_packet_t *)data ;
                                    if( arpResponse == rsp.arp.ar_op ){
                                       printf( "%u.%u.%u.%u => ",
                                               rsp.arp.ar_tip[0],
                                               rsp.arp.ar_tip[1],
                                               rsp.arp.ar_tip[2],
                                               rsp.arp.ar_tip[3] );
                                       for( unsigned i = 0 ; i < sizeof(rsp.arp.ar_tha); i++ ){
                                          if( i )
                                             printf( ":" );
                                          printf( "%02x", rsp.arp.ar_tha[i] );
                                       }
                                       printf( "\n" );
                                    }
                                    else
                                       printf( "not arp response %04x\n", rsp.arp.ar_op );
                                 }
                                 else
                                    printf( "too small: %u of %u\n", payload, sizeof(struct arp_packet_t) );

                                 hexDumper_t dumpRx( data, numRead-(data-responseBuf) );
                                 while( dumpRx.nextLine() ){
                                    printf( "%s\n", dumpRx.getLine() );
                                 }
                              }
                           }
                           else
                              break ;
                        }
		}
		else
			printf( "Error verifying connection\n" );
		close( fdDev );
	}
	else
		perror( devName );
	return 0 ;
}

