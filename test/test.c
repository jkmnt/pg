#define __TEST_C__

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>

#include "pg_syntax.h"
#include "pg_config.h"
#include "pg_primitives.h"
#include "socket.h"
#include "pg_tagitems.h"
#include "pg.h"
#include "test.h"
#include "pg_res.h"
#include "pg_mempool.h"
// #include "widgets/pbar.h"
// #include "widgets/bitmap.h"
// #include "widgets/listbox.h"
// #include "widgets/txtlabel.h"
#include "widgets/group.h"
//#include "widgets/scrolls.h"
// #include "widgets/matrix.h"

#include "res/cap3.xbm"
#include "res/wall.xbm"
#include "res/icon.xbm"
#include "res/disk.xbm"
#include "res/arrows.xbm"
#include "res/msg.xbm"
#include "res/scissors.xbm"
#include "res/f04b_03.xbm"
#include "res/f04b_03b.xbm"
#include "res/f04b_09.xbm"
#include "res/f04b_11.xbm"
#include "res/f04b_19.xbm"
#include "res/f04b_20.xbm"
#include "res/f04b_24.xbm"
#include "res/f04b_25.xbm"
#include "res/f04b_30.xbm"
#include "res/f04b_31.xbm"
#include "res/amiga4ever_pro2.xbm"

PG_DEFINE_PFONT( f04b_03, f04b_03, 0);
PG_DEFINE_PFONT( f04b_03b, f04b_03b, 0);
PG_DEFINE_PFONT( f04b_09, f04b_09, 0);
PG_DEFINE_PFONT( f04b_11, f04b_11, 0);
PG_DEFINE_PFONT( f04b_19, f04b_19, 0);
PG_DEFINE_PFONT( f04b_20, f04b_20, 0);
PG_DEFINE_PFONT( f04b_24, f04b_24, 0);
PG_DEFINE_PFONT( f04b_25, f04b_25, 0);
PG_DEFINE_PFONT( f04b_30, f04b_30, 0);
PG_DEFINE_PFONT( f04b_31, f04b_31, 0);
PG_DEFINE_PFONT( amiga4ever_pro2, amiga4ever_pro2, 0);
PG_DEFINE_BMP( cap3, cap3);
PG_DEFINE_BMP( wall, wall);
PG_DEFINE_BMP( icon, icon);
PG_DEFINE_BMP( disk, disk);
PG_DEFINE_BMP( arrows, arrows);
PG_DEFINE_BMP( msg, msg);
PG_DEFINE_BMP( scissors, scissors);

static const pg_xbm_t *Xbms[] =
{
    &cap3,
    &wall,
    &icon,
    &arrows,
    &disk,
    &msg,
    &scissors,
};

static const pg_font_t *Fonts[] =
{
    &pg_unifont,
    &pg_unifont,
    &f04b_03,
    &f04b_03b,
    &f04b_09,
    &f04b_11,
    &f04b_19,
    &f04b_20,
    &f04b_24,
    &f04b_25,
    &f04b_30,
    &f04b_31,
    &amiga4ever_pro2,
};

#define NTEMS    127 - ' '

static pg_theme_t testTheme =
{
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};

static pg_theme_t testMTheme =
{
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};


static pg_theme_t testTheme1 =
{
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};

static pg_theme_t BorderlessTheme =
{
    { 0, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
    { 0, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
};

static void dbg_hook( pg_obj_t *Obj)
{
    printf("-%d-", Obj->usr_data);
}

static void delay_ms( uint Ms)
{
    Sleep(Ms);
}

static void test_update( void)
{
    send_fb() ;
    clear_fb() ;
    delay_ms( 200) ;
}

static void test_render( void)
{
    printf("Rendering.\n");
    if (pg_render())
        send_fb() ;
    delay_ms( 150) ;
}

static void test_brez( void)
{
    pg_set_clip_via_area( &(pg_fast_area_t) {10, 10, 90, 90}) ;

    int X = 0;
    int Y = 0;
    for ( ; X<128; X++)
    {
        pg_draw_line( 63, 32, X, Y, 1) ;
        test_update() ;
    }
    for ( ; Y<128; Y++)
    {
        pg_draw_line( 63, 32, X, Y, 1) ;
        test_update() ;
    }
    for ( ; X ; X--)
    {
        pg_draw_line( 63, 32, X, Y, 1) ;
        test_update() ;
    }
    for ( ; Y ; Y--)
    {
        pg_draw_line( 63, 32, X, Y, 1) ;
        test_update() ;
    }
}

static void test_fill( void)
{
    pg_set_clip_via_area( &(pg_fast_area_t) {0, 0, 127, 127}) ;
    pg_draw_vline( 0, 0, 31, 1);
    int X=0;
    for ( ; X<128; X++)
    {
        pg_draw_fill( 0, 0, X, X, 1) ;
        test_update() ;
    }
}

static void test_frames( void)
{
    pg_set_clip_via_area( &(pg_fast_area_t) { 0, 0, 90, 90}) ;
    int X=0;
    for ( ; X<128; X++)
    {
        pg_draw_frame( 0, 0, X, X, 1) ;
        test_update() ;
    }
}

static void test_bmp( void)
{
    pg_set_clip_via_area( &(pg_fast_area_t) {10, 10, 90, 90}) ;
    int Start = 31 ;
    for ( int Lim = Start ; Lim < 128 ; Lim++)
    {
         pg_draw_bitmap( &cap3, 0, Start, -1, Lim, 1) ;
         pg_draw_line( 0, Lim, 16, Lim, 1) ;
         pg_draw_line( 100, Lim, 127, Lim, 1) ;
         test_update();
    }
    for ( int Lim = 0 ; Lim < 128 ; Lim++)
    {
         pg_draw_bitmap( &cap3, 0, Lim, -1, -1, 1) ;
         pg_draw_line( 0, Lim, 16, Lim, 1) ;
         pg_draw_line( 100, Lim, 127, Lim, 1) ;
         test_update();
    }
}

static void test_text( void)
{

    for ( int Lim = 0 ; Lim < 128 ; Lim++)
    {
        pg_draw_textline( &pg_unifont, "Simple test string", Lim, 0, -1, -1, 1) ;
        pg_draw_textline( &pg_unifont, "01234567890", Lim, 30, -1, -1, 1) ;
        test_update();
    }
}

static void test_icons( void)
{
    int Start = 31 ;
    for ( int Lim = Start ; Lim < 128 ; Lim++)
    {
         pg_draw_bitmap( &icon, Lim, 0, -1, -1, 1) ;
         //pg_draw_line( 0, Lim, 16, Lim, 1) ;
         //pg_draw_line( 100, Lim, 127, Lim, 1) ;
         test_update();
    }
    for ( int Lim = Start ; Lim < 128 ; Lim++)
    {
         pg_draw_bitmap( &icon, 0, Start, -1, Lim, 1) ;
         pg_draw_line( 0, Lim, 16, Lim, 1) ;
         pg_draw_line( 100, Lim, 127, Lim, 1) ;
         test_update();
    }
    for ( int Lim = 0 ; Lim < 128 ; Lim++)
    {
         pg_draw_bitmap( &icon, 0, Lim, -1, -1, 1) ;
         pg_draw_line( 0, Lim, 16, Lim, 1) ;
         pg_draw_line( 100, Lim, 127, Lim, 1) ;
         test_update();
    }
}

static pg_obj_p *test_div( pg_obj_p *Parent, uint Offset, int X0, int Y0, int X1, int Y1, int Div, uint *Cnt)
{
    int W = (X1 - X0 )/Div - 4;
    int H = (Y1 - Y0)/Div - 4;

    pg_obj_p *Obj = Parent;

    for ( int X = X0 + 2; (X + W) < X1; X+= W + 2)
    {
        for ( int Y = Y0 + 2; (Y + W) < Y1; Y+= H + 2)
        {
            char Str[8];
            sprintf( Str, "%d", (*Cnt)++) ;
            *(++Obj) = pg_create_via_tags( PG_O_BTXTLAB, *(Parent - Offset),

                                           X,
                                           Y,
                                           X + W,
                                           Y + H,

                                           PG_A_TEXT, (pg_val_t) Str,
                                           PG_A_LMARG, 1,
                                           PG_A_TMARG, 1,
                                           PG_A_RMARG, 1,
                                           PG_A_BMARG, 1,
                                           PG_A_USER_DATA, *Cnt-1,
                                           PG_A_PDHOOK, (pg_val_t) dbg_hook);

//             test_render();
        }
    }
    return Obj;
}


static void test_killboxes( uint rmPos)
{
    pg_obj_p Objs[256] = { pg_screen};
    pg_obj_p *curObj = Objs;

    int W = 128;
    int H = 128;
    int Div = 2;

    uint Offset = 0 ;
    uint Cnt = 1;

    *(++curObj) = pg_create_via_tags( PG_O_TXTLAB, pg_screen,
                                      1,
                                      1,
                                      126,
                                      126,
                                      PG_A_TEXT, (pg_val_t) "Pretty big and nasty line which should nicely wrap inside the client window. And here \nwill be the forced newline.",
                                      PG_A_LMARG, 2,
                                      PG_A_TMARG, 2,
                                      PG_A_USER_DATA, 0,
                                      PG_A_PDHOOK, (pg_val_t) dbg_hook);

    for ( ; Div < 8 ; W = W / Div - 2, H = H /Div - 2, Div *= 2, Offset = (Offset + 1) & 3 )
    {
        curObj = test_div( curObj, Offset, 0, 0, W - 1, H - 1, Div, &Cnt);
    }

    printf(" have a %d objects (excluding root) ", curObj - Objs);

    test_render();

    if ( rmPos <= (curObj - Objs)  )
    {
        printf(" killing %d ", rmPos - 1) ;
        pg_kill( Objs[rmPos]) ;
    }

    test_render() ;

    pg_kill( pg_screen) ;
}


static void test_hideboxes( uint hidPos)
{
    pg_obj_p Objs[256] = { pg_screen};
    pg_obj_p *curObj = Objs;

    int W = 128;
    int H = 128;
    int Div = 2;

    uint Offset = 0 ;
    uint Cnt = 1;

    *(++curObj) = pg_create_via_tags( PG_O_BMP, pg_screen,

                                      1,
                                      1,
                                      126,
                                      126,

                                      PG_A_BMP, (pg_val_t) &cap3,
                                      PG_A_USER_DATA, 0,
                                      PG_A_PDHOOK, (pg_val_t) dbg_hook);

    for ( ; Div < 8 ; W = W / Div - 2, H = H /Div - 2, Div *= 2, Offset = (Offset + 1) & 3 )
    {
        curObj = test_div( curObj, Offset, 0, 0, W - 1, H - 1, Div, &Cnt);
    }

    printf(" have a %d objects (excluding root) ", curObj - Objs);

    test_render();

    if ( hidPos <= (curObj - Objs)  )
    {
        printf(" hiding %d ", hidPos - 1) ;
        pg_set( Objs[hidPos], PG_A_HIDDEN, 1) ;
        test_render();
        printf(" unhiding %d ", hidPos - 1) ;
        pg_set( Objs[hidPos], PG_A_HIDDEN, 0) ;
        test_render();
    }

    printf(" removing") ;
    pg_kill( pg_screen) ;
}

// static void test_fonts( void)
// {
//     taglist_t Tl = {{   A_TXTLAB_FONT, (tagdata_t) get_font(2)},
//                     {   A_TXTLAB_TEXT, (tagdata_t) "Hello. Simple f04b_03 font" },
//                     {   A_TXTLAB_XADD, 2},
//                     {   A_TXTLAB_YADD, 0},
//                     {   A_BORD, 0},
//                     {   0 }} ;
//     pg_create_via_taglist( PGO_TXTLAB, Screen,  2, 2, 256, 9,
//                            Tl) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  10,
//                         IA_RIGHT,256,
//                         IA_BOT,  17,
//                         A_TXTLAB_FONT, get_font(3), A_TXTLAB_TEXT, "Simple f04b_03b font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD, 0, A_BORD,0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  18,
//                         IA_RIGHT,256,
//                         IA_BOT,  25,
//                         A_TXTLAB_FONT, get_font(4), A_TXTLAB_TEXT, "Simple f04b_09 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  26,
//                         IA_RIGHT,256,
//                         IA_BOT,  33,
//                         A_TXTLAB_FONT, get_font(5), A_TXTLAB_TEXT, "Simple f04b_11 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  34,
//                         IA_RIGHT,256,
//                         IA_BOT,  48,
//                         A_TXTLAB_FONT, get_font(6), A_TXTLAB_TEXT, "Simple f04b_19 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  49,
//                         IA_RIGHT,256,
//                         IA_BOT,  68,
//                         A_TXTLAB_FONT, get_font(10), A_TXTLAB_TEXT, "Simple f04b_30 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  69,
//                         IA_RIGHT,256,
//                         IA_BOT,  78,
//                         A_TXTLAB_FONT, get_font( fntDefProp), A_TXTLAB_TEXT, "ֿנמסעמי pixcyr2 רנטפע - 1234567890", A_TXTLAB_XADD, 0, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  79,
//                         IA_RIGHT,256,
//                         IA_BOT,  90,
//                         A_TXTLAB_FONT, get_font(7), A_TXTLAB_TEXT, "Simple f04b_20 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  91,
//                         IA_RIGHT,256,
//                         IA_BOT,  97,
//                         A_TXTLAB_FONT, get_font(8), A_TXTLAB_TEXT, "Simple f04b_24 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  98,
//                         IA_RIGHT,256,
//                         IA_BOT,  109,
//                         A_TXTLAB_FONT, get_font(9), A_TXTLAB_TEXT, "Simple f04b_25 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0, A_BORD, 0 ) ;
//
//     test_render() ;
//     delay_ms(2000);
//
//     pg_create_via_tags( PGO_TXTLAB, Screen, 0,
//                         IA_LEFT, 2,
//                         IA_TOP,  110,
//                         IA_RIGHT,256,
//                         IA_BOT,  120,
//                         A_TXTLAB_FONT, get_font(11), A_TXTLAB_TEXT, "Simple f04b_31 font", A_TXTLAB_XADD, 2, A_TXTLAB_YADD,  0,  A_BORD, 0 ) ;
//
//     test_render() ;
//     pg_kill( Screen);
//     //pg_obj_p Tb1 = pg_create_va( pgTextBox, Root, aTop, 0, aBot, 10, A_TXTLAB_FONT, &f04b_03, A_TXTLAB_STR, "Simple f04b_03 fonts", aLeft, 0, aRight, 127, aMargin, aTxtColor, 1 ) ;
// }

static void test_kill( void)
{
    for ( uint Pos = 1; Pos < 22 ; Pos++)
        test_killboxes( Pos);
}

static void test_hide( void)
{
    for ( uint Pos = 1; Pos < 22 ; Pos++)
        test_hideboxes( Pos);
}

static void test_bar( void)
{
    pg_obj_t *Bar1 = pg_create_via_tags( PG_O_HTBAR, pg_screen, 0, 0, 107, 12, PG_A_PERCENT, 12);
    pg_obj_t *Bar2 = pg_create_via_tags( PG_O_HTBAR, pg_screen, 0, 18, 60, 40, PG_A_PERCENT, 0);
    pg_obj_t *Bar3 = pg_create_via_tags( PG_O_VTBAR, pg_screen, 0, 42, 50, 120, PG_A_PERCENT, 0);
    pg_obj_t *Bar4 = pg_create_via_tags( PG_O_HTBAR, Bar3, 2, 8, 44, 22, PG_A_LMARG, 2, PG_A_TMARG, 2, PG_A_RMARG, 2, PG_A_BMARG, 2, PG_A_PERCENT, 0);

    test_render();

    pg_set_via_tags( Bar1, PG_A_CUSTOM_TEXT,  1, PG_A_TEXT, (pg_val_t) "Funky bars in action");

    for ( uint Per = 0; Per <= 100; Per++)
    {
        pg_set( Bar1, PG_A_PERCENT,  Per) ;
        pg_set( Bar2, PG_A_PERCENT,  Per) ;
        pg_set( Bar3, PG_A_PERCENT,  100 - Per) ;
        pg_set( Bar4, PG_A_PERCENT,  Per) ;
        pg_move( Bar2, 2 , 2) ;
        pg_move( Bar1, 1 , 1) ;

        test_render();
    }
    for ( int Per = 100; Per >= 0; Per-=2)
    {
        pg_set( Bar1, PG_A_PERCENT,  Per) ;
        pg_set( Bar2, PG_A_PERCENT,  Per) ;
        pg_set( Bar3, PG_A_PERCENT,  100 - Per) ;
        pg_set( Bar4, PG_A_PERCENT,  Per) ;
        pg_move( Bar1, -5 , -2) ;
        pg_move( Bar3, 1, 2) ;
        test_render();
    }
    pg_kill( pg_screen);
}

static void test_lift_lower( void)
{

    pg_taglist_t Tl1 =
    {
        {PG_A_TEXT, (pg_val_t) "Mojo"},
        {PG_TAG_END, 0},
    };
    pg_taglist_t Tl2 =
    {
        {PG_A_JUST, PG_JUST_C},
        {PG_TAG_NEXT_TAGLIST, (pg_val_t) Tl1},
        {PG_TAG_END, 0},
    };
    pg_taglist_t Tl3 =
    {
        {PG_A_THEME, (pg_val_t) &testTheme1},
        {PG_A_THEME, (pg_val_t) &testTheme1},
        {PG_TAG_NEXT_TAGLIST, (pg_val_t) Tl2},
        {PG_TAG_END, 0},
    };


    pg_obj_t *Gr = pg_create_via_tags( PG_O_TXTLAB, pg_screen, 0, 0, 100, 100, PG_A_TEXT, (pg_val_t)"Lifting and lowering", PG_A_USER_DATA, 0, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab1 = pg_create_via_tags( PG_O_TXTLAB, Gr, 10, 10, 50, 50, PG_A_TEXT,(pg_val_t) "Lab1", PG_A_USER_DATA, 1, PG_A_PDHOOK, (pg_val_t) dbg_hook, PG_TAG_NEXT_TAGLIST, (pg_val_t) Tl3);
    pg_obj_t *Lab2 = pg_create_via_tags( PG_O_TXTLAB, Gr, 15, 16, 55, 56, PG_A_TEXT,(pg_val_t) "Lab2", PG_A_USER_DATA, 2, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab3 = pg_create_via_tags( PG_O_TXTLAB, Gr, 20, 22, 60, 62, PG_A_TEXT,(pg_val_t) "Lab3", PG_A_USER_DATA, 3, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab4 = pg_create_via_tags( PG_O_TXTLAB, Gr, 25, 28, 65, 68, PG_A_TEXT,(pg_val_t) "Lab4", PG_A_USER_DATA, 4, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab5 = pg_create_via_tags( PG_O_TXTLAB, Gr, 30, 34, 70, 74, PG_A_TEXT,(pg_val_t) "Lab5", PG_A_USER_DATA, 5, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab6 = pg_create_via_tags( PG_O_TXTLAB, Gr, 35, 40, 75, 80, PG_A_TEXT,(pg_val_t) "Lab6", PG_A_USER_DATA, 6, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab7 = pg_create_via_tags( PG_O_TXTLAB, Gr, 40, 46, 80, 86, PG_A_TEXT,(pg_val_t) "Lab7", PG_A_USER_DATA, 7, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab8 = pg_create_via_tags( PG_O_TXTLAB, Gr, 45, 52, 85, 92, PG_A_TEXT,(pg_val_t) "Lab8", PG_A_USER_DATA, 8, PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_create_via_tags( PG_O_BOX, Lab1, 10, 25, 20, 28, PG_A_USER_DATA, 11, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab2, 10, 25, 20, 28, PG_A_USER_DATA, 22, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab3, 10, 25, 20, 28, PG_A_USER_DATA, 33, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab4, 10, 25, 20, 28, PG_A_USER_DATA, 44, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab5, 10, 25, 20, 28, PG_A_USER_DATA, 55, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab6, 10, 25, 20, 28, PG_A_USER_DATA, 66, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab7, 10, 25, 20, 28, PG_A_USER_DATA, 77, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab8, 10, 25, 20, 28, PG_A_USER_DATA, 88, PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_obj_t *Arr[] = { /*Gr,*/ Lab1, Lab2, Lab3, Lab4, Lab5, Lab6, Lab7, Lab8};
    int Pos = 0;
    test_render() ;
    int Cnt = 8;
    while ( Cnt--)
    {
        test_render();
        for ( uint i = 0; i<8; i++)
        {
            pg_lift( Arr[Pos]) ;
            test_render();
        }
        test_render();
        for ( uint i = 0; i<8; i++)
        {
            pg_lower( Arr[Pos]) ;
            test_render();
        }
        Pos++ ;
        Pos &= 7;
    }
    pg_kill( pg_screen);
}

static void test_terminal( void)
{
    char Buf1[257] = {0};
    char Buf2[33] = {0};
    int Pos1 = 0 ;
    int Pos2 = 0 ;
    pg_obj_t *Term = pg_create_via_tags( PG_O_TXTLAB, pg_screen, 0, 0, 127, 127, PG_A_TEXT, (pg_val_t) Buf1) ;
    pg_obj_t *Term2 = pg_create_via_tags( PG_O_TXTLAB, Term, 40, 40, 82, 82, /*A_BG_COL, 0, A_FG_COL, 1, A_BORD_COL, 1, A_TXTLAB_FONT, get_font(9),*/ PG_A_TEXT, (pg_val_t) Buf2) ;

    printf("Please type something:\n");
    sprintf( Buf1, "Please type something at console") ;
    pg_render() ;
    send_fb() ;

    char Char ;
    while ( (Char = _getche()) != '\e')
    {
        if (Char == '\r')
            Char = '\n' ;

        Buf1[Pos1++] = Char;
        Buf1[Pos1] = 0 ;
        Buf2[Pos2++] = Char;
        Buf2[Pos2] = 0 ;
        pg_force_update( Term) ;
        pg_force_update( Term2) ;
        pg_render() ;
        send_fb() ;
        Pos1 &= 255 ;
        Pos2 &= 31 ;
    }
}

static void test_bmp_clip( void)
{
    for ( int Lim = 0 ; Lim < 128 ; Lim++)
    {
        pg_set_clip_via_area( &(pg_fast_area_t) {0, 0, Lim, Lim}) ;
        pg_draw_bitmap( &cap3, 0, 0, -1, -1, 1) ;
        test_update();
    }
    for ( int Lim = 0 ; Lim < 128 ; Lim++)
    {
        pg_set_clip_via_area( &(pg_fast_area_t) { Lim, Lim, 127, 127}) ;
        pg_draw_bitmap( &cap3, 0, 0, -1, -1, 1) ;
        test_update();
    }
    for ( int Lim = 0 ; Lim < (128 - 32) ; Lim++)
    {
        pg_set_clip_via_area( &(pg_fast_area_t) { Lim, Lim, Lim + 32, Lim + 32}) ;
        pg_draw_bitmap( &cap3, 16, 16, -1, -1, 1) ;
        test_update();
    }
}

static void test_move( void)
{
    pg_obj_t *Wallpaper = pg_create_via_tags( PG_O_BMP, pg_screen, 0,0, -1, -1,
                                        PG_A_BMP, (pg_val_t) &wall,
                                        PG_A_USER_DATA, 666,
                                        PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_obj_t *Gr = pg_create_via_tags( PG_O_BMP, Wallpaper, 0, 0, 100, 100,
                                     PG_A_BMP, (pg_val_t) &cap3,
                                     PG_A_USER_DATA, 0,
                                     PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_obj_t *Lab1 = pg_create_via_tags( PG_O_TXTLAB, Gr, 10, 10, 50, 50,
                                   PG_A_TEXT, (pg_val_t) "Lab1",
                                   PG_A_USER_DATA, 1,
                                   PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_obj_t *Lab2 = pg_create_via_tags( PG_O_TXTLAB, Gr, 15, 16, 55, 56,
                                   PG_A_TEXT, (pg_val_t) "Lab2",
                                   PG_A_USER_DATA, 2,
                                   PG_A_PDHOOK, (pg_val_t) dbg_hook);

    pg_obj_t *Lab3 = pg_create_via_tags( PG_O_TXTLAB, Gr, 20, 22, 60, 62, PG_A_TEXT, (pg_val_t) "Lab3", PG_A_USER_DATA, 3, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab4 = pg_create_via_tags( PG_O_TXTLAB, Gr, 25, 28, 65, 68, PG_A_TEXT, (pg_val_t) "Lab4", PG_A_USER_DATA, 4, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab5 = pg_create_via_tags( PG_O_TXTLAB, Gr, 30, 34, 70, 74, PG_A_TEXT, (pg_val_t) "Lab5", PG_A_USER_DATA, 5, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab6 = pg_create_via_tags( PG_O_TXTLAB, Gr, 35, 40, 75, 80, PG_A_TEXT, (pg_val_t) "Lab6", PG_A_USER_DATA, 6, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_obj_t *Lab7 = pg_create_via_tags( PG_O_TXTLAB, Gr, 40, 46, 80, 86, PG_A_TEXT, (pg_val_t) "Lab7", PG_A_USER_DATA, 7, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    //pg_obj_t *Lab8 = pg_create_va( pgLabel, Gr, 45, 52, 85, 92, A_STR,"Lab8", A_TAG, 8, A_HOOK, dbg_hook);

    pg_create_via_tags( PG_O_BOX, Lab1, 10, 25, 20, 28, PG_A_USER_DATA, 11, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab2, 10, 25, 20, 28, PG_A_USER_DATA, 21, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab3, 10, 25, 20, 28, PG_A_USER_DATA, 31, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab4, 10, 25, 20, 28, PG_A_USER_DATA, 41, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab5, 10, 25, 20, 28, PG_A_USER_DATA, 51, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab6, 10, 25, 20, 28, PG_A_USER_DATA, 61, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab7, 10, 25, 20, 28, PG_A_USER_DATA, 71, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    //pg_create_va( PGO_BOX, Lab8, 10, 25, 20, 28, A_TAG, 81, A_HOOK, dbg_hook);

    pg_create_via_tags( PG_O_BOX, Lab1, 20, 35, 30, 38, PG_A_USER_DATA, 12, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab2, 20, 35, 30, 38, PG_A_USER_DATA, 22, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab3, 20, 35, 30, 38, PG_A_USER_DATA, 32, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab4, 20, 35, 30, 38, PG_A_USER_DATA, 42, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab5, 20, 35, 30, 38, PG_A_USER_DATA, 52, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab6, 20, 35, 30, 38, PG_A_USER_DATA, 62, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    pg_create_via_tags( PG_O_BOX, Lab7, 20, 35, 30, 38, PG_A_USER_DATA, 72, PG_A_PDHOOK, (pg_val_t) dbg_hook);
    //pg_create_va( PGO_BOX, Lab8, 20, 35, 30, 38, A_TAG, 82, A_HOOK, dbg_hook);


    pg_obj_t *Arr[] = { Gr, Lab1, Lab2, Lab3, Lab4, Lab5, Lab6, Lab7 /*, Lab8*/};
    int Pos = 0;
    test_render() ;
    int Cnt = 20;
    while ( Cnt--)
    {
        int dX = rand() % 10;
        int dY = rand() % 10;
        int dirX = 3 - (rand() % 7);
        int dirY = 3 - (rand() % 7);

        while (1)
        {
            pg_move( Arr[Pos], dirX , dirY) ;
            test_render();

            if (dX)
                dX-- ;

            if (dY)
                dY--;

            if ( !dX && !dY)
                break;
        }

        Pos++ ;
        Pos &= 7;
    }
    pg_kill( pg_screen);
}

static void tst_print( pg_obj_t *Obj, int Num)
{
    const char * Strings[] =
    {
        "Zero","One","Two","Three","Four","Five","Six","Seven","Eight","Nine","Ten",
        "Eleven","Twelve","Thirteen","Fourteen","Fithteen","Sixteen","Seventeen","Eighteen","Nineteen",
        "Twenty", "Twenty-one","Twenty-two", "",
    };

    pg_set_via_tags( Obj, PG_A_TEXT, (pg_val_t) Strings[Num]);
}

static void tst_print_sel_icon( pg_obj_t *Obj, int Num)
{
    pg_set_via_tags( Obj, PG_A_BMP, (pg_val_t) Xbms[4 + (Num % 3)]);
}

void test_list( void)
{
    static pg_theme_t listTestTheme =
    {
        { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
        { PG_F_BORD  | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_BLACK, PG_CL_WHITE, PG_CL_BLACK, &pg_unifont},
        { 0/*/fBord | fBordPush */, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
    };

    listTestTheme.norm.font = Fonts[12];

    static pg_theme_t listBorderlessTheme =
    {
        { 0, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
        { PG_F_BORD | PG_F_PUSH_BORD | PG_F_ROUNDED, PG_CL_WHITE, PG_CL_BLACK, PG_CL_BLACK, &pg_unifont},
        { 0, PG_CL_WHITE, PG_CL_WHITE, PG_CL_WHITE, &pg_unifont},
    };

    pg_taglist_t itemInit =
    {
        {PG_A_JUST, PG_JUST_C},
        {PG_A_THEME, (pg_val_t) &listTestTheme},
        {PG_TAG_END,         0},
    };

    pg_create_via_tags( PG_O_TXTLAB, pg_screen, 0, 0, -1, 9, PG_A_JUST, PG_JUST_C, PG_A_TMARG, 0, PG_A_TEXT, (pg_val_t) "Use w s a d and Esc keys");

    pg_obj_t *Lb = pg_create_via_tags(  PG_O_HLISTBOX, pg_screen, 0, 90, 120, 126,
                                        PG_A_CITEM, 18,
                                        PG_IA_NSLOTS, 7,
                                        PG_A_NITEMS, 7,
                                        PG_IA_SLOT_PAD, 1,
                                        PG_IA_SLOT_LEN, 22,
                                        PG_IA_BMARG, 8,
                                        PG_A_UPD_HOOK, (pg_val_t) tst_print_sel_icon,
                                        PG_IA_SLOT_TYPE, PG_O_BMP,
                                        PG_A_THEME, (pg_val_t) &listBorderlessTheme,
                                        PG_A_SLOT_TAGLIST, (pg_val_t) itemInit);

    pg_obj_t *Lb2 = pg_create_via_tags( PG_O_VLISTBOX, pg_screen, 20, 12, 106, 98,
                                        PG_A_CITEM, 3,
                                        PG_IA_NSLOTS, 7,
                                        PG_IA_SLOT_LEN, 12,
                                        PG_A_NITEMS, 22,
                                        PG_IA_SLOT_PAD, 1,
                                        PG_IA_LMARG, 5,
                                        PG_IA_RMARG, 2,
                                        PG_IA_TMARG, 4,
                                        PG_IA_BMARG, 4,
                                        PG_A_UPD_HOOK, (pg_val_t) tst_print,
                                        PG_A_SLOT_TAGLIST, (pg_val_t) itemInit,
                                        PG_IA_SLOT_TYPE, PG_O_BTXTLAB);

    pg_obj_t *Scr1 = pg_create_via_tags( PG_O_HSCROLL, Lb, 2, 34, 115, 0, 0);
    pg_obj_t *Scr2 = pg_create_via_tags( PG_O_VSCROLL, Lb2, 0, 0, 3, 0, 0);

    pg_set( Lb, PG_A_SCROLLER, (pg_val_t) Scr1);
    pg_set( Lb2, PG_A_SCROLLER, (pg_val_t) Scr2);

    test_render();

    while (1)
    {
        int Char;
        switch ( Char = _getche())
        {
        case 'w':   pg_act( Lb2, PG_M_CHANGE_SEL, -1); break;
        case 'W':   pg_act( Lb2, PG_M_CHANGE_SEL, -5); break;

        case 's':   pg_act( Lb2, PG_M_CHANGE_SEL, 1); break;
        case 'S':   pg_act( Lb2, PG_M_CHANGE_SEL, 5); break;

        case 'a':   pg_act( Lb, PG_M_CHANGE_SEL, -1); break;
        case 'A':   pg_act( Lb, PG_M_CHANGE_SEL, -5); break;

        case 'd':   pg_act( Lb, PG_M_CHANGE_SEL, 1); break;
        case 'D':   pg_act( Lb, PG_M_CHANGE_SEL, 5); break;

        case '\e':  goto _end;
        }
        test_render();
        pg_act( Lb2, PG_M_DIRTY, 0);
    }
//     int Cnt = 4;
//     while ( Cnt--)
//     {
//         for ( int i = 21; i; i--)
//         {
//             pg_set( Lb, A_AITEM, i % 7);
//             //pg_act( Lb2,  M_REDRAW_ITEM, 7);
//             pg_act( Lb2, M_CHANGE_SEL, 1);
//             test_render();
//         }
//         for ( int i = 0; i< 22; i++)
//         {
//              pg_set( Lb, A_AITEM, i % 5);
//              pg_act( Lb2, M_CHANGE_SEL, -1);
//              test_render();
//         }
//     }
    _end:
    pg_kill( pg_screen);
}

static void upd_gr_item( pg_obj_t *Me, int Pos)
{
    char Buf[4];
    sprintf( Buf, "%d!", Pos);
    pg_set(Me, PG_A_TEXT, (pg_val_t) Buf);
}

static void test_group( void)
{
    pg_taglist_t initTags = {  {PG_A_TEXT, (pg_val_t)"!!!"},
                            {PG_A_JUST, PG_JUST_C},
                            {PG_TAG_END, 0}};

    pg_gr_item_t Items[] =
    {
        { PG_O_BTXTLAB, {1, 1, 10, 10}, initTags, upd_gr_item},
        { PG_O_BTXTLAB, {12, 1, 23, 10}, initTags, 0},
        { PG_O_BTXTLAB, {25, 1, 34, 14}, initTags, upd_gr_item},
        { PG_O_BTXTLAB, {36, 1, 46, 15}, initTags, 0},
        { PG_O_BTXTLAB, {48, 1, 58, 16}, initTags, upd_gr_item},
        { PG_O_BTXTLAB, {60, 1, 70, 17}, initTags, 0},
    };

    pg_create_via_tags( PG_O_GROUP, pg_screen, 0, 3, 50, 17,
                        PG_IA_NITEMS, countof( Items),
                        PG_IA_ITEMDATA, (pg_val_t) Items);

    pg_print_tree();

    test_render();
}

static void test_listgroup( void)
{
    pg_taglist_t initTags = {  {PG_A_TEXT, (pg_val_t)"!!!"},
                            {PG_A_JUST, PG_JUST_C},
                            {PG_TAG_END, 0}};

    pg_gr_item_t Items[] =
    {
        { PG_O_BTXTLAB, {1, 2, 10, 11},  initTags, upd_gr_item},
        { PG_O_BTXTLAB, {12, 2, 23, 13}, initTags, 0},
        { PG_O_BTXTLAB, {25, 2, 34, 15}, initTags, upd_gr_item},
        { PG_O_BTXTLAB, {36, 2, 46, 16}, initTags, 0},
        { PG_O_BTXTLAB, {48, 2, 58, 17}, initTags, upd_gr_item},
        { PG_O_BTXTLAB, {60, 2, 70, 18}, initTags, 0},
    };

    pg_taglist_t slotInit =
    {
       { PG_IA_NITEMS, countof( Items)},
       { PG_IA_ITEMDATA, (pg_val_t) Items},
       { PG_TAG_END, 0},
    };

    pg_obj_t *Lb2 = pg_create_via_tags( PG_O_VLISTBOX, pg_screen, 0, 20, 127, 112,
                                    PG_A_CITEM, 3,
                                    PG_IA_NSLOTS, 7,
                                    PG_IA_SLOT_LEN, 26,
                                    PG_A_NITEMS, 22,
                                    PG_IA_SLOT_PAD, 2,
                                    PG_A_SLOT_TAGLIST, (pg_val_t) slotInit,
                                    PG_IA_SLOT_TYPE, PG_O_GROUP);

    pg_print_tree();

    test_render();
    while ( 1)
    {
        for ( int i = 21; i; i-- )
        {
            pg_act( Lb2, PG_M_CHANGE_SEL, 1);
            test_render();
        }
        for ( int i = 21; i; i-- )
        {
            pg_act( Lb2, PG_M_CHANGE_SEL, -1);
            test_render();
        }
    }
}

static void upd_item( pg_obj_t *Me, int Pos)
{
    char Buf[4];
    //sprintf( Buf, "%d", Pos);
    sprintf( Buf, "%c", Pos + ' ');
    pg_set_via_tags(Me, /*A_TXTLAB_FONT_ID, Pos,*/
                    PG_A_TEXT, (pg_val_t) (Pos < NTEMS ? Buf : ""),
                    PG_A_THEME, (pg_val_t) (Pos < NTEMS ? &testTheme : &BorderlessTheme));
}

static void test_matrix( void)
{
    testTheme.norm.font = Fonts[12];
    testTheme1.norm.font = Fonts[2];

    pg_taglist_t itemInit =
    {
        {PG_A_JUST, PG_JUST_C},
        //{A_THEME, (tagdata_t) &testTheme},
        {PG_TAG_END,         0},
    };

    pg_obj_t* dbgWnd = pg_create_via_tags( PG_O_BTXTLAB, pg_screen, 0, 2, 110, 25,
                                            PG_A_JUST, PG_JUST_C,
                                            // A_MULTILINE_TEXT, 1,
                                            PG_A_THEME, (pg_val_t) &testTheme1,
                                            PG_A_TEXT, (pg_val_t) "Use w/s/a/d cursor");


    pg_obj_t *Mx = pg_create_via_tags(  PG_O_MATRIX, pg_screen, 0, 30, 122, 127,
                                        PG_A_CITEM, 'a' - ' ',
                                        PG_A_NITEMS, NTEMS,
                                        PG_IA_NCOLS, 6,
                                        PG_IA_NROWS, 9,
                                        PG_IA_SLOT_H, 14,
                                        PG_IA_SLOT_W, 14,
                                        PG_IA_LMARG, 5,
//                                         IA_SLOT_XPAD, 11,
//                                         IA_SLOT_YPAD, 11,
                                        PG_A_UPD_HOOK, (pg_val_t) upd_item,
                                        PG_IA_SLOT_TYPE, PG_O_BTXTLAB,
                                        PG_A_THEME, (pg_val_t) &pg_def_theme,
                                        PG_A_SLOT_TAGLIST, (pg_val_t) itemInit,
                                        PG_A_WRAP_CURSOR, 1,
                                        PG_A_USE_SCROLLING, 1
//                                        A_PDHOOK, draw_left_vscroll,
                                        );

    pg_obj_t *Scr1 = pg_create_via_tags( PG_O_VSCROLL, Mx, 0, 0, 3, -1, 0);
    pg_set( Mx, PG_A_SCROLLER, (pg_val_t) Scr1);

    test_render();
    delay_ms(2000);
//     int Cnt = 4;
//     while ( Cnt--)
//     {
//         for ( int i = NTEMS - 1; i > 0; i--)
//         {
//             pg_act( Mx, M_CHANGE_SEL, 1);
//             test_render();
//         }
//         for ( int i = 0; i< NTEMS; i++)
//         {
//              pg_act( Mx, M_CHANGE_SEL, -1);
//              test_render();
//         }
    while (1)
    {
        char Buf[20];
        int Char;
        switch ( Char = _getche())
        {
        case 'w':   pg_act( Mx, PG_M_PREV_ROW, 0);   break;
        case 's':   pg_act( Mx, PG_M_NEXT_ROW, 0);   break;
        case 'a':   pg_act( Mx, PG_M_PREV_COL, 0);   break;
        case 'd':   pg_act( Mx, PG_M_NEXT_COL, 0);   break;
        case '\e':  goto _end;
        }
        uint aItem, aCol, aRow;
        pg_get( Mx, PG_A_CITEM, &aItem);
        pg_get( Mx, PG_ROA_COL, &aCol);
        pg_get( Mx, PG_ROA_ROW, &aRow);
        sprintf( Buf, "item %d, col %d, row %d", aItem, aCol, aRow);
        pg_set( dbgWnd, PG_A_TEXT, (pg_val_t) Buf);
        test_render();
    }
    _end:
    pg_kill( pg_screen);
}


static void test_scroll( void)
{
    pg_obj_t *Scr1 = pg_create_via_tags( PG_O_VSCROLL, pg_screen, 0, 0, 120, 13,
                                                      PG_A_LMARG, 2,
                                                      PG_A_TMARG, 2,
                                                      PG_A_RMARG, 2,
                                                      PG_A_BMARG, 2,
                                                      PG_A_NITEMS, 200,
                                                      PG_A_CITEM, 0,
                                                      PG_A_PAGELEN, 7
                                                      //A_THEME, &BorderlessTheme,
                                                      );

    pg_obj_t* dbgWnd = pg_create_via_tags( PG_O_BTXTLAB, pg_screen, 0, 20, 110, 50,
                                            PG_A_JUST, PG_JUST_C,
                                            // A_MULTILINE_TEXT, 1,
                                            PG_A_THEME, (pg_val_t) &testTheme1,
                                            PG_A_TEXT, (pg_val_t) "Scrolls in action");

    //pg_obj_t *Bar2 = pg_create_via_tags( PGO HTBAR, Screen, 0, 18, 60, 40, A_XMARG, 1, A_YMARG, 1, A_PERCENT, 0);
//     pg_obj_t *Bar3 = pg_create_via_tags( PGO_VTBAR, Screen, 0, 42, 50, 120, A_XMARG, 1, A_YMARG, 1, A_PERCENT, 0);
//     pg_obj_t *Bar4 = pg_create_via_tags( PGO_HTBAR, Bar3, 2, 8, 44, 22, A_XMARG, 2, A_YMARG, 2, A_PERCENT, 0);
//
    test_render();

    char Buf[64];
    uint nItems, cItem, pageLen;

    while (1)
    {
        for ( uint Per = 0; Per <= 200; Per++)
        {
            pg_set( Scr1, PG_A_CITEM,  Per) ;

            pg_get( Scr1, PG_A_NITEMS, &nItems);
            pg_get( Scr1, PG_A_CITEM, &cItem);
            pg_get( Scr1, PG_A_PAGELEN, &pageLen);
            sprintf( Buf, "item %d of %d (%d per page)", cItem, nItems, pageLen);
            pg_set( dbgWnd, PG_A_TEXT, (pg_val_t) Buf);

            test_render();
            //delay_ms(200);
        }
        for ( int Per = 200; Per >= 0; Per-=2)
        {
            pg_set( Scr1, PG_A_CITEM,  Per) ;

            pg_get( Scr1, PG_A_NITEMS, &nItems);
            pg_get( Scr1, PG_A_CITEM, &cItem);
            pg_get( Scr1, PG_A_PAGELEN, &pageLen);
            sprintf( Buf, "item %d of %d (%d per page)", cItem, nItems, pageLen);
            pg_set( dbgWnd, PG_A_TEXT, (pg_val_t) Buf);

            test_render();
            //delay_ms(200);
        }
    }
    pg_kill( pg_screen);
}

static int canvPos;

static void canvas_paint( pg_obj_t *Me)
{
    int X0 = Me->area.left;
    int X1 = Me->area.right;
    int Y0 = Me->area.top;
    int Y1 = Me->area.bot;

    pg_draw_line( (X0 + X1)/2, (Y0 + Y1)/2, X0 + canvPos, Y1, Me->theme->norm.fgcol) ;
}

static void test_canvas( void)
{
    pg_obj_t *Canv = pg_create_via_tags( PG_O_CANVAS, pg_screen, 10, 10, 50, 50,
                        PG_A_PDHOOK, (pg_val_t) canvas_paint);

    for (canvPos = 0; canvPos < 127; canvPos++)
    {
        pg_act( Canv, PG_M_DIRTY, 0);
        pg_act( pg_screen, PG_M_DIRTY, 0);
        test_render();
    }
}

static void test_line_breaks( void)
{
    testTheme.norm.font = Fonts[0];

    const char *Txt = "\nAnother  black suit wearing MAN holds hands with a RED-HEADED GIRL in a prep school uniform. They look through the beer cooler in the back of the store. Both girls are around seventeen."
        "\nThe two men in black suits are the notorious Abilene bank robbers, SETH and RICHARD GECKO, 'The Gecko Brothers.' ";

//  const char *Txt = "*\n**An\nother*black*suit*wearing*MAN*holds*hands*with*a*RED-HEADED*GIRL*in*a*prep*school*uniform.*They*look*through*the*beer*cooler*in*the*back*of*the*store.*Both*girls*are*around*seventeen."
//          "\n\nThe*two*men*in*black*suits*are*the*notorious*Abilene*bank*robbers,*SETH*and*RICHARD*GECKO,*'The*Gecko*Brothers.'*";

    int i = 2;

    while (1)
    {
        int Char;
        switch ( Char = _getche())
        {
        case 'w':   i++; break;
        case 'W':   i+=10; break;
        case 's':   i--; break;
        case 'S':   i-=10; break;

        case '\e':  return;
        }

        if ( i>128 )
            i = 128;
        if ( i<2 )
            i=2;
        pg_kill( pg_screen);

        pg_create_via_tags( PG_O_TXTLAB, pg_screen, 0, 0, i, 128,
                            PG_A_THEME, (pg_val_t) &testTheme,
                            PG_A_TEXT, (pg_val_t) Txt,
                            PG_A_USER_DATA, 0,
                            PG_A_PDHOOK, (pg_val_t) dbg_hook);

        test_render();
    }

//  for ( int i=2; i<120; i++ )
//  {
//      const char *Txt = "Another black suit wearing MAN holds hands with a RED-HEADED GIRL in a prep school uniform. They look through the beer cooler in the back of the store. Both girls are around seventeen."
//          "\nThe two men in black suits are the notorious Abilene bank robbers, SETH and RICHARD GECKO, 'The Gecko Brothers.' ";

//          ""
//          "And the other customers are all being held hostage. Seth is the "
//          "one with the prep girl. Richard is the one with the blonde. \nEverybody speaks low and fast.";

//        test_render();
//      pg_kill( Screen);
//      delay_ms(200);
//  }

}

void do_demo( void)
{
//  test_list();
//  set_clip_via_area( 0, 0, 250, 130) ;
//  test_bmp();
//  delay_ms(5000);
//  test_bmp_clip();
//  delay_ms(5000);
//  test_icons();
//  delay_ms(5000);
//  test_bmp();
//  delay_ms(5000);
//  test_canvas();
//  delay_ms(5000);
//  test_line_breaks();
//  delay_ms(5000);
//  test_listgroup();
//    test_line_breaks();
//    test_move();
//    delay_ms(5000);
////    test_fonts() ;
////    delay_ms(8000);
//    test_lift_lower();
//    delay_ms(5000);
    test_kill();
    delay_ms(5000);
    test_hide();
    delay_ms(5000);
    test_bar();
    delay_ms(5000);
    test_move();
    delay_ms(5000);
    test_list();
    delay_ms(5000);
    test_matrix();
    delay_ms(5000);
    test_terminal();
}

int main( void)
{
    srand( time(0));
    init_fb();
    do_demo();
    printf("Exit");
    pg_print_mempool_stat();
}
