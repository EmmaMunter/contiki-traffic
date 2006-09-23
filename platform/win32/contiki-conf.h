#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_FASTCALL      __fastcall



#if _USRDLL
#define CCIF __declspec(dllimport)
#else
#define CCIF __declspec(dllexport)
#endif
#define CLIF __declspec(dllexport)



#define strcasecmp _stricmp



#define LOG_CONF_ENABLED 1
#include "sys/log.h"
CCIF void debug_printf(char *format, ...);



#include <time.h>
#define CLOCK_CONF_SECOND CLK_TCK
typedef unsigned long clock_time_t;



#include <ctype.h>
#define ctk_arch_isprint isprint

typedef char ctk_arch_key_t;
#include "ctk/ctk-console.h"

#define CH_ULCORNER	          0xDA
#define CH_URCORNER	          0xBF
#define CH_LLCORNER	          0xC0
#define CH_LRCORNER	          0xD9
#define CH_ENTER	          '\r'
#define CH_DEL		          '\b'
#define CH_CURS_UP  	          -1
#define CH_CURS_LEFT	          -2
#define CH_CURS_RIGHT	          -3
#define CH_CURS_DOWN	          -4

#define CTK_CONF_MENU_KEY         -5  /* F10 */
#define CTK_CONF_WINDOWSWITCH_KEY -6  /* Ctrl-Tab */
#define CTK_CONF_WIDGETDOWN_KEY   '\t'
#define CTK_CONF_WIDGETUP_KEY     -7  /* Shift-Tab */
#define CTK_CONF_MOUSE_SUPPORT    1
#define CTK_CONF_ICONS            1
#define CTK_CONF_ICON_BITMAPS     0
#define CTK_CONF_ICON_TEXTMAPS    1
#define CTK_CONF_WINDOWMOVE       1
#define CTK_CONF_WINDOWCLOSE      1
#define CTK_CONF_MENUS            1
#define CTK_CONF_MENUWIDTH        16
#define CTK_CONF_MAXMENUITEMS     10
#define CTK_CONF_WIDGET_FLAGS     0
#define CTK_CONF_SCREENSAVER      0

#define COLOR_BLACK	(0)
#define COLOR_BLUE	(1)
#define COLOR_GRAY	(1 | 2 | 4)
#define COLOR_CYAN	(1 | 2 | 8)
#define COLOR_YELLOW	(2 | 4 | 8)
#define COLOR_WHITE	(1 | 2 | 4 | 8)

#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK
#define BACKGROUNDCOLOR     COLOR_BLACK
#define WINDOWCOLOR_FOCUS   COLOR_WHITE  | COLOR_BLUE * 0x10
#define WINDOWCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define DIALOGCOLOR         COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_HLINK   COLOR_CYAN   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FWIN    COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_DIALOG  COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW | COLOR_BLUE * 0x10
#define MENUCOLOR           COLOR_WHITE  | COLOR_BLUE * 0x10
#define OPENMENUCOLOR       COLOR_WHITE  | COLOR_BLUE * 0x10
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW | COLOR_BLUE * 0x10



#define EMAIL_CONF_WIDTH 76
#define EMAIL_CONF_HEIGHT 25



#define IRC_CONF_WIDTH 78
#define IRC_CONF_HEIGHT 30
#define IRC_CONF_SYSTEM_STRING "Win32"



#define LOADER_CONF_ARCH "loader/dll-loader.h"

#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 10
#define PROGRAM_HANDLER_CONF_QUIT_MENU   1



#define SHELL_GUI_CONF_XSIZE 46
#define SHELL_GUI_CONF_YSIZE 22



typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     420
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP_CHECKSUMS   1



#define WWW_CONF_WEBPAGE_WIDTH      76
#define WWW_CONF_WEBPAGE_HEIGHT     30
#define WWW_CONF_HISTORY_SIZE       40
#define WWW_CONF_MAX_URLLEN         200
#define WWW_CONF_MAX_NUMPAGEWIDGETS 80
#define WWW_CONF_RENDERSTATE        1
#define WWW_CONF_FORMS              1
#define WWW_CONF_MAX_FORMACTIONLEN  200
#define WWW_CONF_MAX_FORMNAMELEN    200
#define WWW_CONF_MAX_INPUTNAMELEN   200
#define WWW_CONF_MAX_INPUTVALUELEN  240

#endif /* __CONTIKI_CONF_H__ */
