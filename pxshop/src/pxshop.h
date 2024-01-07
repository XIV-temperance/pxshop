#define _GNU_SOURCE
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bmptools.h"

#define SETTINGSFILE "settings.cfg"
#define PALETTEDIR "palettes"
#define D_DARKMODE 0
#define D_AUTOSAVE 0
#define D_SIZE 16

#define MAXCOLORS 249 // Unfortunate ncurses limitation

#define PREVIEW 250
#define REDLINE 251
#define RED 252
#define WHITE 253
#define BLACK 254
#define GREY 255

#define PXMODE 0
#define LINEMODE 1
#define FILLMODE 2

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define KEY_CTRL(x) ((x) & 0x1F)
#define READ_SETTING(x) ((x) & 0x0F)
#define WRITE_SETTING(x) ((x) | 0x30)
#define GET_THEME(x) (x) ? BLACK : WHITE

// Globals for catching our user input
int CANVX, CANVY, CURX, CURY;
short SINDEX, SELECTOR[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// Globals for our instance settings
int WIDTH, HEIGHT;
unsigned char BITRATE, DARKMODE = D_DARKMODE, AUTOSAVE = D_AUTOSAVE;
char D_PALETTE[FILENAME_MAX + 1] = {'\0'};
bool UNSAVED;

// Linked list of palette entries
typedef struct PALETTECOLOR{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	struct PALETTECOLOR *next;
}PALETTECOLOR;
int PALSIZE;
PALETTECOLOR *PALLIST = NULL;

// Linked list of available palettes
typedef struct PALETTEFILE{
	char name[FILENAME_MAX + 1];
	struct PALETTEFILE *next;
}PALETTEFILE;

// UI drawing
void draw_borders(void);
void draw_title(void);
void draw_heading(char *heading);
void draw_controls(char *controls[], int numControls);
void draw_box(int startX, int startY, int endX, int endY, bool fill);
void draw_warning(char *warning[]);
void draw_canvas(unsigned char canvas[]);
void draw_button(char *text, int x, int y, int selected, int pressed);
int draw_popup(int W, int H);

// Main menu
void main_menu(char *filename, unsigned char canvas[]);
void draw_main_menu(unsigned char canvas[], int brushMode);
void draw_cursor(void);
void draw_selector(int brushMode);

// Controls menu
void controls_menu(unsigned char canvas[]);
void draw_controls_menu(unsigned char canvas[]);

// Settings menu
void settings_menu(unsigned char canvas[], char *fileName);
void draw_settings_menu(unsigned char canvas[], int selX, int selY);
void settings_update_buttons(int selX, int selY, int menW, int menH);

// File browser menu
void browser_menu(unsigned char canvas[], int mode);
void draw_browser_menu(unsigned char canvas[], int sel, int numPals, PALETTEFILE *fileList);
void draw_browser_list(int menW, int menH, int sel, int numPals, PALETTEFILE *fileList);

// Palette menu
void palette_menu(unsigned char canvas[]);
void draw_palette_menu(unsigned char canvas[], int sel, int swap);
void draw_selected_color(int sel);
void draw_big_selector(int menW, int menH);
void draw_palette_preview(int menW, int menH, int sel, int swap);

// Colorpicker menu
void colorpicker_menu(unsigned char canvas[], int sel);
void draw_colorpicker_menu(unsigned char canvas[], int oColor, int sVal, unsigned char redV, unsigned char greenV, unsigned char blueV);
void colorpicker_update(int menW, int menH, int oColor, int sVal, unsigned char redV, unsigned char greenV, unsigned char blueV);

// Painting
short get_color(int x, int y);
void pick_color(unsigned char canvas[]);
int xy_to_canvas(int x, int y);
void paint_color(unsigned char canvas[], int x, int y);
void flood_fill(unsigned char canvas[], int x, int y, short oldColor, bool checkArray[]);
void paint_bucket(unsigned char canvas[]);

// Data processing
void load_settings(void);
int load_bitmap(BITMAP *bmp, char *filename);
void load_bmp_palette(BITMAP bmp);
int import_palette(char *filename);
void load_default_palette(void);
void init_new_palette(void);
int generate_pfile_list(PALETTEFILE **fileList);
int count_colors(unsigned char canvas[], int color);
int count_pfile_colors(PALETTEFILE *fileList, int index);

// Data updating
void update_bitrate(void);
void update_colors(void);
void new_color(int r, int g, int b);
void swap_colors(int colorX, int colorY);
void delete_color(unsigned char canvas[], int color);

// Data writing
int write_settings(void);
int write_file(char *filename, unsigned char canvas[]);
int write_palette(char *filename);

// Misc
int lstring(char *array[], int arraySize);
void sbfstring(char *buffer, char *string, int maxLength);
void clamp_positions(void);
void free_palette_list(PALETTECOLOR *list);
void free_file_list(PALETTEFILE **list);
int check_extension(char *fileName, char * extension);