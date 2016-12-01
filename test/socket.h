#ifndef __SOCKET_H__
#define __SOCKET_H__
/** Public */

extern void init_fb(void);
extern void send_fb( void);
extern void clear_fb(void);

#ifdef __SOCKET_C__
/** Private */

#define WINSOCKVERSION    MAKEWORD( 2,2 )
#define HOST        "127.0.0.1"
//#define HOST        "192.168.1.5"
#define PORT         50007

#endif // !__SOCKET_C__
#endif // !__SOCKET_H__
