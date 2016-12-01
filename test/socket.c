#define __SOCKET_C__

#include <sys/types.h>
#include <Winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "socket.h"
#include "drv_fb_mono_32.h"

static int connectionFd;

const pg_framebuffer_t *fb;

struct
{
    uint16_t w;
    uint16_t h;
} txHeader = {.w = _SCR_W, .h = _SCR_H};

void send_fb( void)
{
    printf( "Sending\n");
    while ( 1)
    {
        if ( send( connectionFd, (char *) &txHeader, sizeof(txHeader), 0 ) == SOCKET_ERROR)
        {
            printf( "Retry on header\n");
            continue;
        }

        if ( send( connectionFd, (char *) fb->raw_bytes, sizeof(fb->raw_bytes), 0 ) == SOCKET_ERROR)
        {
            printf( "Retry on data\n");
            continue;
        }
        break;
    }
}

int open_socket(void)
{
    int rc;
    struct sockaddr_in servAddr, localAddr;
    WSADATA wsaData;
    if( WSAStartup( WINSOCKVERSION, &wsaData) != 0 )
        exit(ERROR);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr(HOST);

      // Create socket
    connectionFd = socket(AF_INET, SOCK_STREAM, 0);

    /* bind any port number */
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(connectionFd,
      (struct sockaddr *) &localAddr, sizeof(localAddr));

    // Connect to Server
    if ( connect(connectionFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        exit(ERROR);

    return 1;
}

void init_fb(void)
{
    fb = pg_open_framebuffer();
    open_socket();
    printf("Opened socket");
}

void clear_fb(void)
{
    pg_clear_framebuffer();
}
