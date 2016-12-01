#define __TEST_BW_DRIVER_C__

#include "pg_syntax.h"
#include <sys/types.h>
#include <Winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "test_bw_driver.h"
#include "pg.h"

#include "pg_res.h"
#include "res/f04b_11.xbm"
#include "drv_fb_mono_32.h"

//PG_DEFINE_PFONT(f04b_11, f04b_11, 0);

struct
{
    uint16_t w;
    uint16_t h;
} txHeader = {.w = _SCR_W, .h = _SCR_H};

enum
{
    CMD_CLEAR,
    CMD_SHOW,
    CMD_SET_CLIP,
    CMD_DRAW_VLINE,
    CMD_DRAW_HLINE,
    CMD_DRAW_LINE,
    CMD_DRAW_FILL,
    CMD_DRAW_FRAME,
    CMD_DRAW_TEXTLINE,
};

const pg_framebuffer_t *fb;

static int listsock, newsock;

int error(const char * msg)
{
    printf("%s: %d", msg, WSAGetLastError());
    exit(1);
}

void open_socket(void)
{
    struct sockaddr_in serv_addr, cli_addr;
    WSADATA wsaData;
    if( WSAStartup( WINSOCKVERSION, &wsaData))
        error("Failed to start winsock");

    listsock = socket(AF_INET, SOCK_STREAM, 0);
    if (listsock == SOCKET_ERROR)
        error("ERROR opening socket");

    memset(&serv_addr, sizeof(serv_addr), 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // bind
    bind(listsock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listsock, SOMAXCONN) == SOCKET_ERROR)
        error("ERROR on listen");

    int clilen = sizeof(cli_addr);

    newsock = accept(listsock, (struct sockaddr *) &cli_addr, &clilen);

    if (newsock == SOCKET_ERROR)
        error("ERROR on accept");
}

void ack(int sock)
{
    send(sock, "Ok", sizeof("Ok"), 0);
}

void send_fb( int sock)
{
    printf( "Sending\n");
    while ( 1)
    {
        if ( send( sock, (char *) &txHeader, sizeof(txHeader), 0 ) == SOCKET_ERROR)
        {
            printf( "Retry on header\n");
            continue;
        }

        if ( send( sock, (char *) fb->raw_bytes, sizeof(fb->raw_bytes), 0 ) == SOCKET_ERROR)
        {
            printf( "Retry on data\n");
            continue;
        }
        break;
    }
}

void process_command(const uint32_t *buf)
{
    switch (buf[0])
    {
        case CMD_CLEAR:
            pg_clear_framebuffer();
            break;

        case CMD_SET_CLIP:
            pg_set_clip_via_area( &(pg_fast_area_t) {buf[1], buf[2], buf[3], buf[4]});
            break;

        case CMD_DRAW_VLINE:
            pg_draw_vline(buf[1], buf[2], buf[3], buf[4]);
            break;

        case CMD_DRAW_HLINE:
            pg_draw_hline(buf[1], buf[2], buf[3], buf[4]);
            break;

        case CMD_DRAW_LINE:
            pg_draw_line(buf[1], buf[2], buf[3], buf[4], buf[5]);
            break;

        case CMD_DRAW_FILL:
            pg_draw_fill(buf[1], buf[2], buf[3], buf[4], buf[5]);
            break;

        case CMD_DRAW_FRAME:
            pg_draw_frame(buf[1], buf[2], buf[3], buf[4], buf[5]);
            break;

        case CMD_DRAW_TEXTLINE:
            pg_draw_textline(&pg_unifont, "Test string", buf[1], buf[2], 250, 131, buf[3]);
            break;

        case CMD_SHOW:
            send_fb(newsock);
            return;

        default:
            printf("Got trash\n");
            break;
    }
    ack(newsock);
}

int get_command(void)
{
    uint32_t inbuf[256];

    if (recv(newsock, (char*) inbuf, sizeof(inbuf), 0) <= 0)
        return 0;

    process_command(inbuf);

    return 1;
}

int main(void)
{
    fb = pg_open_framebuffer();
    while (1)
    {
        printf("listening\n");
        open_socket();

        while (get_command());

        closesocket(newsock);
        closesocket(listsock);
    }
}
