		/*  Version 1.0 of PxShop
			Made by Marcus S. (Temperance-XIV)  */

#include "pxshop.h"

int main(int argc, char *argv[]){
	// Print help
	if (argc == 1 || argc == 3 || argc > 4){
		printf("Usage:\n");
		printf("opening:    ./pxshop filename.bmp\n");
		printf("new file:   ./pxshop newfilename.bmp width height\n");
		return 2;
	}
	// Read the settings file (if it exists)
	load_settings();
	// Check file exists and validate it
	BITMAP bmp = {0};
	if (argc == 2){
		int validate = load_bitmap(&bmp, argv[1]);
		if (validate)
			return validate;
		WIDTH = bmp.info.Width;
		HEIGHT = bmp.info.Height;
		load_bmp_palette(bmp);
		BITRATE = bmp.info.Bitrate;
	}
	// Check file is new and update globals with params
	if (argc == 4){
		if (access(argv[1], F_OK) == 0){
			printf("File already exists. Overwrite? (y/n) ");
			char input = 0;
			scanf("%c", &input);
			if (input != 'y' && input != 'Y')
				return 2;
		}
		if (!check_extension(argv[1], "bmp")){
			printf("Can only create bitmap (.bmp) files\n");
			return 2;
		}
		WIDTH = atoi(argv[2]);
		HEIGHT = atoi(argv[3]);
		if (WIDTH < 1)
			WIDTH = D_SIZE;
		if (HEIGHT < 1)
			HEIGHT = D_SIZE;
		init_new_palette();
	}
	// Create our canvas and populate it
	unsigned char canvas[WIDTH * HEIGHT];
	if (argc == 2){
		bmp_to_array(bmp, canvas);
		fclose(bmp.bmpFile);
		}
	if (argc == 4){
		for (int i = 0; i < WIDTH * HEIGHT; i++)
			canvas[i] = 0;
	}

	// ncurses initialization
	initscr();
	start_color();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	// Abort if terminal doesn't support colors
	if (!can_change_color() || COLORS < 256){
		free_palette_list(PALLIST);
		endwin();
		printf("Terminal environment incompatible\n");
		return 1;
	}

	// State initialization
	update_colors();
	CURX = COLS / 2;
    CURY = LINES / 2;
	CANVX = COLS / 2 - WIDTH;
	CANVY = LINES / 2 + HEIGHT / 2;
	clamp_positions();
	bkgd(COLOR_PAIR(GREY));

	// Begin!
	main_menu(argv[1], canvas);

	// Exit
	free_palette_list(PALLIST);
	endwin();
	return 0;
}


// UI drawing
void draw_borders(void){
    move(0, 0);
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	hline(' ', COLS);
	move(1, 0);
	hline(' ', COLS);
	move(2, 0);
	hline(' ', COLS);
	move(3, 0);
	vline(' ', LINES);
	move(3, COLS - 1);
	vline(' ', LINES);
	move(LINES - 2, 0);
	hline(' ', COLS);
	move(LINES - 1, 0);
	hline(' ', COLS);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	return;
}

void draw_title(void){
	char *title = ".::PxShop  V1.0::.";
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	move(0, (COLS - strlen(title)) / 2);
	printw("%s", title);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
}

void draw_heading(char *heading){
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	move(1, (COLS - strlen(heading)) / 2);
	printw("%s", heading);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
}

void draw_controls(char *controls[], int numControls){
	char *sizeWarning[] = {
		"Resize terminal",
		"to see controls"
		};
	int line1padding = COLS, line2padding = COLS;
	bool tooBig = false;
	for (int i = 0; i < numControls / 2; i++){
		line1padding -= strlen(controls[i]);
		if (line1padding < 0){
			line1padding = 0;
			tooBig = true;
		}
	}
	for (int i = numControls / 2; i < numControls; i++){
		line2padding -= strlen(controls[i]);
		if (line2padding < 0){
			line2padding = 0;
			tooBig = true;
		}
	}
	line1padding /= numControls;
	line2padding /= numControls;
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	if (tooBig){
		move(LINES - 2, (COLS - strlen(sizeWarning[0])) / 2);
		printw("%s", sizeWarning[0]);
		move(LINES - 1, (COLS - strlen(sizeWarning[1])) / 2);
		printw("%s", sizeWarning[1]);
	}else{
		move(LINES - 2, 1);
		for (int i = 0; i < numControls / 2; i++){
			printw("%s", controls[i]);
			for (int j = 0; j < line1padding; j++)
				printw(" ");
		}
		move(LINES - 1, 1);
		for (int i = numControls / 2; i < numControls; i++){
			printw("%s", controls[i]);
			for (int j = 0; j < line2padding; j++)
				printw(" ");
		}
	}
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	return;
}

void draw_box(int startX, int startY, int endX, int endY, bool fill){
	if (fill){
		for (int i = 0; i < endY - startY; i++){
			move(startY + i, startX);
			hline(' ', endX - startX);
		}
	}
	move(startY, startX);
	hline(ACS_HLINE, endX - startX);
	vline(ACS_VLINE, endY - startY);
	addch(ACS_ULCORNER);
	move(endY, startX);
	hline(ACS_HLINE, endX - startX);
	addch(ACS_LLCORNER);
	move(startY, endX);
	vline(ACS_VLINE, endY - startY);
	addch(ACS_URCORNER);
	move(endY, endX);
	addch(ACS_LRCORNER);
	return;
}

void draw_warning(char *warning[]){
	attron(COLOR_PAIR(RED));
	move(LINES - 2, 0);
	hline(' ', COLS);
	move(LINES - 1, 0);
	hline(' ', COLS);
	char *sizeWarning[] = {
		"Resize terminal",
		"to see message "
		};
	int line1padding = (COLS - strlen(warning[0])) / 2; 
	int line2padding = (COLS - strlen(warning[1])) / 2;
	bool tooBig = false;
	if (line1padding < 0 || line2padding < 0){
		line1padding = 0;
		line2padding = 0;
		tooBig = true;
	}
	if (tooBig){
		move(LINES - 2, (COLS - strlen(sizeWarning[0])) / 2);
		printw("%s", sizeWarning[0]);
		move(LINES - 1, (COLS - strlen(sizeWarning[1])) / 2);
		printw("%s", sizeWarning[1]);
	}else{
		move(LINES - 2, 1);
		for (int i = 0; i < line1padding; i++)
			printw(" ");
		printw("%s", warning[0]);
		move(LINES - 1, 1);
		for (int i = 0; i < line2padding; i++)
			printw(" ");
		printw("%s", warning[1]);
	}
	attroff(COLOR_PAIR(RED));
	return;
}

void draw_canvas(unsigned char canvas[]){
	for (int y = 0; y < HEIGHT; y++){
		int altX = CANVX;
		int altY = CANVY - y;
		for (int x = 0; x < WIDTH; x++){
			attron(COLOR_PAIR(canvas[x + WIDTH * y] + 1));
			if (altX > 0 && altY > 2 && altX < COLS - 2 && altY < LINES - 2){
				mvaddstr(altY, altX, "  ");
			}
			altX += 2;
			attroff(COLOR_PAIR(canvas[x + WIDTH * y] + 1));
		}
	}
	return;
}

int draw_popup(int W, int H){
	char *warning[] = {"Resize",
					   "Window"};
	int fits = 1;
	if (COLS - W - 4 < 0 || LINES - H - 8 < 0)
		fits = 0;
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	if (fits){
		for (int i = (LINES - H) / 2; i < (LINES - H) / 2 + H; i++){
			move(i, (COLS - W) / 2);
			hline(' ', W);
		}
	}else{
		for (int i = 0; i < ARRAY_SIZE(warning); i++){
			move(LINES / 2 + i, (COLS - strlen(warning[i])) / 2);
			printw(" %s ", warning[i]);
		}
	}
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	return fits;
}

void draw_button(char *text, int x, int y, int selected, int pressed){
	move(y, x);
	if (pressed)
		attron(COLOR_PAIR(RED));
	else
		attron(COLOR_PAIR(BLACK - DARKMODE));
	printw("%s", text);
	if (pressed)
		attroff(COLOR_PAIR(RED));
	else
		attroff(COLOR_PAIR(BLACK - DARKMODE));
	if (selected){
		attron(COLOR_PAIR(GET_THEME(DARKMODE)));
		move(y - 1, x - 1);
		addch(ACS_ULCORNER);
		hline(ACS_HLINE, strlen(text));
		move(y - 1, x + strlen(text));
		addch(ACS_URCORNER);
		move(y + 1, x - 1);
		addch(ACS_LLCORNER);
		hline(ACS_HLINE, strlen(text));
		move(y + 1, x + strlen(text));
		addch(ACS_LRCORNER);
		move(y, x - 1);
		addch(ACS_VLINE);
		move(y, x + strlen(text));
		addch(ACS_VLINE);
		attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	}
	return;
}

// Main menu
void main_menu(char *filename, unsigned char canvas[]){
	char *warning1[] = {"Could not write file to disc",
					    "     |space - continue|     "};
	char *warning2[] = {" You have unsaved changes ",
					    "Save before exit?    |y/n|"};
	bool lockCur = false, exitSignal = false;
	int kInput, brushMode = 0;
	draw_main_menu(canvas, brushMode);
	refresh();
	while((kInput = getch()) != 0){
    	switch(kInput){	
			// Cursor movement
            case KEY_LEFT:
            if (!lockCur && CURX > 1)
			    CURX -= 2;
			else CANVX += 2;	
			break;

			case KEY_RIGHT:
            if (!lockCur && CURX < COLS - 4)
			    CURX += 2;
			else CANVX -= 2;	
			break;

			case KEY_UP:
            if (!lockCur && CURY > 3)
			    CURY--;
			else CANVY++;
			break;

			case KEY_DOWN:
            if (!lockCur && CURY < LINES - 3)
			    CURY++;
			else CANVY--;
			break;	

			case KEY_RESIZE:
			erase();
			draw_main_menu(canvas, brushMode);
			break;

			case KEY_CTRL('X'):
			palette_menu(canvas);
			break;

			case KEY_CTRL('Z'):
			settings_menu(canvas, filename);
			break;

			case KEY_CTRL('S'):
			if (UNSAVED){
				if (!write_file(filename, canvas)){
					draw_warning(warning1);
					while((kInput = getch()) != ' '){
						if (kInput == KEY_RESIZE){
							erase();
							draw_main_menu(canvas, brushMode);
							draw_warning(warning1);
						}
					}
				}
				UNSAVED = false;
			}
			break;

			case KEY_CTRL('C'):
			controls_menu(canvas);
			break;

			case KEY_CTRL('L'):
			if (UNSAVED){
				draw_warning(warning2);
				while((kInput = getch()) != 'n'){
					if (kInput == KEY_RESIZE){
						erase();
						draw_main_menu(canvas, brushMode);
						draw_warning(warning2);
					}
					if (kInput == 'y'){
						write_file(filename, canvas);
						UNSAVED = false;
						break;
					}
				}
			}
			exitSignal = true;
			break;

			case 'Q':
			case 'q':
			SINDEX--;
			if (SINDEX < 0){
				if (PALSIZE < 16)
					SINDEX = PALSIZE - 1;
				else
					SINDEX = 15;
			}
			break;

			case 'W':
			case 'w':
			pick_color(canvas);
			break;

			case 'E':
			case 'e':
			SINDEX++;
			if (SINDEX > PALSIZE - 1)
				SINDEX = 0;
			if (SINDEX > 15)
				SINDEX = 0;
			break;

			case 'R':
			case 'r':
			if (lockCur)
				lockCur = false;
			else
				lockCur = true;
			break;

			case ' ':
			if (brushMode == PXMODE || brushMode == LINEMODE)
				paint_color(canvas, CURX, CURY);
			else if (brushMode == FILLMODE)
				paint_bucket(canvas);
			break;

			case 'P':
			case 'p':
			if (brushMode != FILLMODE)
				brushMode = FILLMODE;
			else brushMode = PXMODE;
			break;

			case 'O':
			case 'o':
			if (brushMode != LINEMODE)
				brushMode = LINEMODE;
			else brushMode = PXMODE;
			break;

			case 'T':
			case 't':
			CANVX = COLS / 2 - WIDTH;
			CANVY = LINES / 2 + HEIGHT / 2;
			clamp_positions();
			break;
		}
		if (exitSignal)
			break;
		clamp_positions();
		if (brushMode == LINEMODE)
			paint_color(canvas, CURX, CURY);
		erase();
		draw_main_menu(canvas, brushMode);
        refresh();
		if (UNSAVED && AUTOSAVE){
			if (!write_file(filename, canvas)){
				draw_warning(warning1);
				while((kInput = getch()) != ' '){
					if (kInput == KEY_RESIZE){
						erase();
						draw_main_menu(canvas, brushMode);
						draw_warning(warning1);
					}
				}
			}
			UNSAVED = false;
		}
	}
}

void draw_main_menu(unsigned char canvas[], int brushMode){
	char *controls[] = {
		"^C Controls ",
		"^S Save     ",
		"^X Palette  ",
		"^Z Settings ",
		"^L Exit     ",
		"            "
		};
	draw_canvas(canvas);
	draw_cursor();
	draw_borders();
	draw_title();
	draw_selector(brushMode);
	draw_controls(controls, ARRAY_SIZE(controls));
	return;
}

void draw_cursor(void){
	move(CURY, CURX);
	attron(COLOR_PAIR(get_color(CURX, CURY)));
	attron(A_REVERSE);
	hline(' ', 2);
	attroff(A_REVERSE);
	attroff(COLOR_PAIR(get_color(CURX, CURY)));
	return;
}

void draw_selector(int brushMode){
	int limit = PALSIZE;
	if (limit > ARRAY_SIZE(SELECTOR))
		limit = ARRAY_SIZE(SELECTOR);
	
	move(1, COLS / 2 - limit);
	attron(A_UNDERLINE);
	for (int i = 0; i < limit; i++){
		attron(COLOR_PAIR(SELECTOR[i]));
		addch(ACS_S1);
		addch(ACS_S1);
		attroff(COLOR_PAIR(SELECTOR[i]));
	}
	attroff(A_UNDERLINE);
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	move(2, COLS / 2 - limit);
	for (int i = 0; i < limit; i++){
		if (i == SINDEX)
			printw("%c%c", '^', '^');
		else
			printw("  ");
	}
	printw(" ");
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(COLOR_PAIR(RED));
	switch (brushMode){
		case LINEMODE:
		printw("L");
		break;
		case FILLMODE:
		printw("B");
		break;
	}
	attroff(COLOR_PAIR(RED));
	return;
}

// Controls menu
void controls_menu(unsigned char canvas[]){
	erase();
	draw_controls_menu(canvas);
	int kInput;
	while((kInput = getch()) != KEY_CTRL('C')){
    	switch(kInput){	
			case KEY_RESIZE:
			erase();
			draw_controls_menu(canvas);
			break;
		}
	}
	return;
}

void draw_controls_menu(unsigned char canvas[]){
	char *heading = "Controls  Menu";
	char *controls[] = {" ", "^C exit "};
	char *info[] = {"Q  Previous color   space  Paint pixel",
					"W  Eyedropper       P  Bucket mode    ",
					"E  Next color       O  Line mode      ",
					"R  Toggle camera    T  Recenter camera"};
	int menW = 44, menH = 11;
	// UI drawing
	draw_canvas(canvas);
	draw_borders();
	draw_title();
	draw_heading(heading);
	draw_controls(controls, ARRAY_SIZE(controls));
	if (draw_popup(menW, menH)){
		attron(COLOR_PAIR(GET_THEME(DARKMODE)));
		for (int i = 0; i < ARRAY_SIZE(info); i++){
			move((LINES - menH) / 2 + 2 + i * 2, (COLS - menW) / 2 + 3);
			printw("%s", info[i]);
		}
		attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	}
	return;
}

// Settings menu
void settings_menu(unsigned char canvas[], char *fileName){
	char *warning1[] = {"  Too many colors present to reduce bitrate  ",
					   "Try deleting some first    |space - continue|"};
	char *warning2[] = {"Failed to write settings to disc",
					   "        |space - continue|       "};
	char *warning3[] = {"Failed to write palette to disc",
					   "        |space - continue|      "};
	int selX = 0, selY = 0;
	erase();
	draw_settings_menu(canvas, selX, selY);
	int kInput;
	while((kInput = getch()) != KEY_CTRL('Z')){
    	switch(kInput){	
            case KEY_LEFT:
			if (selX > 0)
				 selX--;
			else switch (selY){
				case 0:
				selX = 3;
				break;
				case 1:
				case 2:
				case 4:
				selX = 1;
				break;
			}
			break;

			case KEY_RIGHT:
			switch (selY){
				case 0:
				if (selX < 3)
					selX++;
				else selX = 0;
				break;
				case 1:
				case 2:
				case 4:
				if (selX == 0)
					selX++;
				else selX = 0;
				break;
			}
			break;

			case KEY_UP:
			selX = 0;
			if (selY == 0)
				selY = 4;
			else selY--;
			break;

			case KEY_DOWN:
			selX = 0;
			if (selY == 4)
				selY = 0;
			else selY++;
			break;	

			case KEY_RESIZE:
			erase();
			draw_settings_menu(canvas, selX, selY);
			break;

			case KEY_CTRL('W'):
			if (!write_settings()){
				draw_warning(warning2);
				while((kInput = getch()) != ' '){
					if (kInput == KEY_RESIZE){
						erase();
						draw_settings_menu(canvas, selX, selY);
						draw_warning(warning2);
					}
				}
			}
			break;

			case ' ':
			switch (selY){
				case 0:
				switch (selX){
					case 0: 
					if (PALSIZE < 3){
						BITRATE = 1;
						UNSAVED = true;
					}else{
						draw_warning(warning1);
						while((kInput = getch()) != ' '){
							if (kInput == KEY_RESIZE){
								erase();
								draw_settings_menu(canvas, selX, selY);
								draw_warning(warning1);
							}
						}
					}
					break;
					case 1: 
					if (PALSIZE < 5){
						BITRATE = 2;
						UNSAVED = true;
					}else{
						draw_warning(warning1);
						while((kInput = getch()) != ' '){
							if (kInput == KEY_RESIZE){
								erase();
								draw_settings_menu(canvas, selX, selY);
								draw_warning(warning1);
							}
						}
					}
					break;
					case 2: 
					if (PALSIZE < 17){
						BITRATE = 4;
						UNSAVED = true;
					}else{
						draw_warning(warning1);
						while((kInput = getch()) != ' '){
							if (kInput == KEY_RESIZE){
								erase();
								draw_settings_menu(canvas, selX, selY);
								draw_warning(warning1);
							}
						}
					}
					break;
					case 3: 
					BITRATE = 8;
					UNSAVED = true;
					break;
				}
				break;

				case 1:
				if (selX == 0)
					 DARKMODE = 0;
				else DARKMODE = 1;
				break;

				case 2:
				if (selX == 0)
					 AUTOSAVE = 1;
				else AUTOSAVE = 0;
				break;

				case 3:
				browser_menu(canvas, 0);
				break;

				case 4:
				if (selX == 0)
					browser_menu(canvas, 1);
				else if (selX == 1){
					if (!write_palette(fileName)){
						draw_warning(warning3);
						while((kInput = getch()) != ' '){
							if (kInput == KEY_RESIZE){
								erase();
								draw_settings_menu(canvas, selX, selY);
								draw_warning(warning3);
							}
						}
					}
				}break;
			}
			break;
		}
		erase();
		draw_settings_menu(canvas, selX, selY);
	}
	return;
}

void draw_settings_menu(unsigned char canvas[], int selX, int selY){
	char *heading = "Settings  Menu";
	char *controls[] = {"space select     ",
						"^W write to disc ",
						"^Z exit          ",
						"                 "};
	int menW = 44, menH = 21;
	draw_canvas(canvas);
	draw_borders();
	draw_title();
	draw_heading(heading);
	draw_controls(controls, ARRAY_SIZE(controls));
	if (draw_popup(menW, menH))
		settings_update_buttons(selX, selY, menW, menH);
	return;
}

void settings_update_buttons(int selX, int selY, int menW, int menH){
	// Figuring out which buttons are pressed
	int pressed[8] = {0}, selected[11] = {0};
	switch (BITRATE){
		case 1: pressed[0] = 1; break;
		case 2: pressed[1] = 1; break;
		case 4: pressed[2] = 1; break;
		case 8: pressed[3] = 1; break;
	}
	if (DARKMODE)
		 pressed[5] = 1;
	else pressed[4] = 1;
	if (AUTOSAVE)
		 pressed[6] = 1;
	else pressed[7] = 1;
	// Figuring out which button is selected
	if (selY == 0){
		switch (selX){
			case 0: selected[0] = 1; break;
			case 1: selected[1] = 1; break;
			case 2: selected[2] = 1; break;
			case 3: selected[3] = 1; break;
		}
	} else if (selY == 1){
		if (selX == 0)
			 selected[4] = 1;
		else selected[5] = 1;
	} else if (selY == 2){
		if (selX == 0)
			 selected[6] = 1;
		else selected[7] = 1;
	} else if (selY == 3){
		selected[8] = 1;
	} else if (selY == 4){
		if (selX == 0)
			 selected[9] = 1;
		else selected[10] = 1;
	}
	// Making sure the default palette's name fits in the screen
	char buffer[21] = {'\0'};
	sbfstring(buffer, D_PALETTE, 20);
	// Printing the buttons
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(A_UNDERLINE);
	move((LINES - menH) / 2 + 2, (COLS - menW) / 2 + 3);
	printw("Bitrate:");
	move((LINES - menH) / 2 + 6, (COLS - menW) / 2 + 3);
	printw("Theme:");
	move((LINES - menH) / 2 + 10, (COLS - menW) / 2 + 3);
	printw("Auto-Save:");
	move((LINES - menH) / 2 + 14, (COLS - menW) / 2 + 3);
	printw("Default Palette:");
	attroff(A_UNDERLINE);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	draw_button(" 1 ", (COLS / 2 - 7), ((LINES - menH) / 2 + 2),  selected[0], pressed[0]);
	draw_button(" 2 ", (COLS / 2 - 1), ((LINES - menH) / 2 + 2),  selected[1], pressed[1]);
	draw_button(" 4 ", (COLS / 2 + 5), ((LINES - menH) / 2 + 2),  selected[2], pressed[2]);
	draw_button(" 8 ", (COLS / 2 + 11), ((LINES - menH) / 2 + 2),  selected[3], pressed[3]);
	draw_button(" light ", (COLS / 2 - 7), ((LINES - menH) / 2 + 6),  selected[4], pressed[4]);
	draw_button(" dark ",  (COLS / 2 + 3), ((LINES - menH) / 2 + 6),  selected[5], pressed[5]);
	draw_button(" enabled ",    (COLS / 2 - 7), ((LINES - menH) / 2 + 10), selected[6], pressed[6]);
	draw_button(" disabled ",   (COLS / 2 + 5), ((LINES - menH) / 2 + 10), selected[7], pressed[7]);
	draw_button(buffer,    (COLS / 2), ((LINES - menH) / 2 + 14), selected[8], 0);
	draw_button(" import palette ",    (COLS / 2 - 18), ((LINES - menH) / 2 + 18), selected[9], 0);
	draw_button(" export palette ",    (COLS / 2 + 2), ((LINES - menH) / 2 + 18), selected[10], 0);
	return;
}

// Palette browser menu
void browser_menu(unsigned char canvas[], int mode){
	// First we must fetch our list of palettes by reading the directory
	PALETTEFILE *fileList = NULL;
	int numPals = generate_pfile_list(&fileList);
	char *warning1[] = {"Selected palette file is corrupt/invalid",
					    "           |space - continue|           "};
	char *warning2[] = {"New palette is smaller, extra colors will be shrunk",
					    "               Continue?    |y / n|                "};
	int sel = 0, kInput;
	bool exitSignal = false;
	erase();
	draw_browser_menu(canvas, sel, numPals, fileList);
	while((kInput = getch()) != KEY_CTRL('Z')){
    	switch(kInput){	
			case KEY_UP:
			if (sel > 0)
				sel--;
			break;

			case KEY_DOWN:
			if (sel < numPals - 1)
				sel++;
			break;	

			case KEY_RESIZE:
			erase();
			draw_browser_menu(canvas, sel, numPals, fileList);
			break;

			case ' ':{
				PALETTEFILE *ptr = fileList;
				for (int i = 0; i < sel; i++)
					ptr = ptr->next;
				int numCols = count_pfile_colors(fileList, sel);
				if (numCols < 2 || numCols > MAXCOLORS){
					draw_warning(warning1);
						while((kInput = getch()) != ' '){
							if (kInput == KEY_RESIZE){
								erase();
								draw_browser_menu(canvas, sel, numPals, fileList);
								draw_warning(warning1);
							}
						}
					break;
				}
				if (mode == 0){
					sprintf(D_PALETTE, "%s", ptr->name);
					exitSignal = true;
				}else if (mode == 1){
					if (numCols < PALSIZE){
						bool skipUpdate = false;
						draw_warning(warning2);
						while((kInput = getch()) != 'y'){
							if (kInput == KEY_RESIZE){
								erase();
								draw_browser_menu(canvas, sel, numPals, fileList);
								draw_warning(warning2);
							}
							if (kInput == 'n')
								skipUpdate = true;
						}
						if (skipUpdate)
							break;
					}
					char fileName[FILENAME_MAX + 1];
					sprintf(fileName, PALETTEDIR"/%s.hex", ptr->name);
					import_palette(fileName);
					update_colors();
					for (int i = 0; i < WIDTH * HEIGHT; i++)
						if (canvas[i] + 1 > numCols)
							canvas[i] %= numCols;
					UNSAVED = true;
					exitSignal = true;
				}
			}
			break;
		}
		erase();
		draw_browser_menu(canvas, sel, numPals, fileList);
		if (exitSignal)
			break;
	}
	free_file_list(&fileList);
	return;
}

void draw_browser_menu(unsigned char canvas[], int sel, int numPals, PALETTEFILE *fileList){
	char *heading = "Palette Browser";
	char *controls[] = {"space select ",
						"^Z Exit      "};
	int menW = 40, menH = 40;
	// Menu can shrink vertically to a certain extent
	if (LINES - menH - 8 < 0)
		menH += LINES - menH - 8;
	if (menH < 15)
		menH = 15;
	// UI drawing
	draw_canvas(canvas);
	draw_borders();
	draw_title();
	draw_heading(heading);
	draw_controls(controls, ARRAY_SIZE(controls));
	if (draw_popup(menW, menH)){
		draw_browser_list(menW, menH, sel, numPals, fileList);
	}
}

void draw_browser_list(int menW, int menH, int sel, int numPals, PALETTEFILE *fileList){
	// Drawing the title
	char title[menW + 1];
	sprintf(title, "Directory: ./"PALETTEDIR);
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(A_UNDERLINE);
	move((LINES - menH) / 2 + 1, (COLS - menW) / 2 + (menW - strlen(title)) / 2);
	printw("%s", title);
	attroff(A_UNDERLINE);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));

	// Drawing the border box
	draw_box((COLS-menW) / 2 + 4, 
			 (LINES - menH) / 2 + 2, 
			 (COLS + menW) / 2 - 5, 
			 (LINES + menH) / 2 - 2, 
			 true);

	// if all the names don't fit, we need scrolling functionality
	int maxScroll = numPals - (menH - 6) / 2;
	int scrolled = 0;
	while (sel >= (menH - 6) / 2 + scrolled)
		scrolled++;
	
	// This adds scrolling indicators on the screen
	if (scrolled < maxScroll){
		move((LINES + menH) / 2 - 2, (COLS - menW) / 2 + (menW - 8) / 2);
		printw("vvvvvvvv");
	}
	if (scrolled > 0){
		move((LINES - menH) / 2 + 2, (COLS - menW) / 2 + (menW - 8) / 2);
		printw("^^^^^^^^");
	}
	attroff(COLOR_PAIR(GREY));
	// Drawing the names out as buttons
	for (int i = scrolled; i < numPals; i ++){
		if (i - scrolled < (menH - 6) / 2){
			move((LINES - menH) / 2 + 4 + (i - scrolled) * 2, (COLS - menW) / 2 + 6);
			attron(COLOR_PAIR(BLACK - DARKMODE));
			PALETTEFILE *ptr = fileList;
			char buffer[menW - 11], fileName[FILENAME_MAX + 1];
			for (int j = 0; j < i; j++)
				ptr = ptr->next;
			sprintf(fileName, "%s - %i", ptr->name, count_pfile_colors(fileList, i));
			sbfstring(buffer, fileName, menW - 12);
			printw("%s", buffer);
			attroff(COLOR_PAIR(BLACK - DARKMODE));
			if (i == sel){ // Drawing a box around the selected name
				attron(COLOR_PAIR(GREY));
				draw_box((COLS - menW) / 2 + 5,
						 (LINES - menH) / 2 + 3 + (i - scrolled) * 2,
						 (COLS - menW) / 2 + 6 + strlen(buffer),
						 (LINES - menH) / 2 + 5 + (i - scrolled) * 2, false);
				attroff(COLOR_PAIR(GREY));
			}
		}
	}
	return;
}

// Palette menu
void palette_menu(unsigned char canvas[]){
	char *warning1[] = {"At color limit, can't create new color",
					    "          |space - continue|          "};
	char *warning2[] = {"Can't have less than 2 colors",
					    "     |space - continue|      "};
	char *warning3[] = {"Color present in image, pixels will be replaced",
					    "             Continue?    |y / n|              "};
	int sel = 0, swap = -1, kInput;
	erase();
	draw_palette_menu(canvas, sel, swap);
	while((kInput = getch()) != KEY_CTRL('X')){
    	switch(kInput){	
            case KEY_LEFT:
			if (sel > 0)
				sel--;
			break;

			case KEY_RIGHT:
			if (sel + 1 < PALSIZE)
				sel++;
			break;

			case KEY_UP:
			if (sel - 16 >= 0)
				sel -= 16;
			break;

			case KEY_DOWN:
			if (sel + 16 < PALSIZE)
				sel += 16;
			break;	

			case KEY_RESIZE:
			erase();
			draw_palette_menu(canvas, sel, swap);
			break;

			case 'Q':
			case 'q':
			SINDEX--;
			if (SINDEX < 0){
				if (PALSIZE < 16)
					SINDEX = PALSIZE - 1;
				else
					SINDEX = 15;
			}
			break;

			case 'E':
			case 'e':
			SINDEX++;
			if (SINDEX > PALSIZE - 1)
				SINDEX = 0;
			if (SINDEX > 15)
				SINDEX = 0;
			break;

			case 'A':
			case 'a':
			if (PALSIZE < MAXCOLORS){
				new_color(255, 255, 255);
				update_colors();
			}else{
				draw_warning(warning1);
				while((kInput = getch()) != ' '){
					if (kInput == KEY_RESIZE){
						erase();
						draw_palette_menu(canvas, sel, swap);
						draw_warning(warning1);
					}
				}
			}
			break;

			case 'S':
			case 's':
			SELECTOR[SINDEX] = sel + 1;
			SINDEX++;
			if (SINDEX > PALSIZE - 1)
				SINDEX = 0;
			if (SINDEX > 15)
				SINDEX = 0;
			break;

			case 'D':
			case 'd':
			if (PALSIZE > 2){
				if (count_colors(canvas, sel + 1)){
					draw_warning(warning3);
					while((kInput = getch()) != 'n'){
						if (kInput == KEY_RESIZE){
							erase();
							draw_palette_menu(canvas, sel, swap);
							draw_warning(warning3);
						}
						if (kInput == 'y'){
							delete_color(canvas, sel + 1);
							update_colors();
							sel--;
							if (sel < 0)
								sel = 0;
							break;
						}
					}
				}else{
					delete_color(canvas, sel + 1);
					update_colors();
					sel--;
					if (sel < 0)
						sel = 0;
				}
			}else{
				draw_warning(warning2);
				while((kInput = getch()) != ' '){
					if (kInput == KEY_RESIZE){
						erase();
						draw_palette_menu(canvas, sel, swap);
						draw_warning(warning2);
					}
				}				
			}
			break;

			case 'F':
			case 'f':
			colorpicker_menu(canvas, sel);
			break;

			case 'G':
			case 'g':
			if (swap == -1)
				swap = sel;
			else if (swap == sel)
				swap = -1;
			else{
				swap_colors(sel, swap);
				update_colors();
				swap = -1;
			}
			break;
		}
		draw_palette_menu(canvas, sel, swap);
	}
	return;
}

void draw_palette_menu(unsigned char canvas[], int sel, int swap){
	char *heading = "Palette Menu";
	char *controls[] = {"A New    ",
						"S Rack   ",
						"D Delete ",
						"F Edit   ",
						"^X Exit  ",
						"Q Prev   ",
						"E Next   ",
						"G Swap   "};
	int menW = 59, menH = 50;
	// Menu can shrink vertically to a certain extent
	if (LINES - menH - 8 < 0)
		menH += LINES - menH - 8;
	if (menH < 15)
		menH = 15;
	// UI drawing
	draw_canvas(canvas);
	draw_borders();
	draw_title();
	draw_heading(heading);
	draw_controls(controls, ARRAY_SIZE(controls));
	if (draw_popup(menW, menH)){
		draw_selected_color(sel);
		draw_big_selector(menW, menH);
		draw_palette_preview(menW, menH, sel, swap);
	}
}

void draw_selected_color(int sel){
	char *text = "Selected color: #";
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	if (sel > 0)
		move(2, (COLS - strlen(text) - (int) log10(sel) - 1) / 2);
	else move(2, (COLS - strlen(text) - 1) / 2);
	printw("%s", text);
	printw("%i", sel + 1);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	return;
}

void draw_big_selector(int menW, int menH){
	// Drawing the title
	char *title = "Palette";
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(A_UNDERLINE);
	move((LINES - menH) / 2 + 1, (COLS - menW) / 2 + (menW - strlen(title)) / 2);
	printw("%s", title);
	attroff(A_UNDERLINE);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));

	// Establishing selector size (if fewer colors than 16)
	int limit = PALSIZE;
	if (limit > 16)
		limit = 16;

	// Drawing the border box
	attron(COLOR_PAIR(GREY));
	draw_box((COLS - menW) / 2 + (menW - limit * 3) / 2 - 1,
			 (LINES - menH) / 2 + 2,
			 (COLS - menW) / 2 + (menW + limit * 3) / 2 + 1,
			 (LINES - menH) / 2 + 6,
			 true);
	attroff(COLOR_PAIR(GREY));

	// Drawing the swatches
	move((LINES - menH) / 2 + 4, (COLS - menW) / 2 + (menW - limit * 3) / 2 + 1);
	for (int i = 0; i < limit; i++){
		attron(COLOR_PAIR(SELECTOR[i]));
		printw("  ");
		attroff(COLOR_PAIR(SELECTOR[i]));
		attron(COLOR_PAIR(GREY));
		printw(" ");
		attroff(COLOR_PAIR(GREY));
	}

	// Drawing the selector box
	attron(COLOR_PAIR(GREY));
	draw_box((COLS - menW) / 2 + (menW - limit * 3) / 2 + SINDEX * 3,
			 (LINES - menH) / 2 + 3,
			 (COLS - menW) / 2 + (menW - limit * 3) / 2 + SINDEX * 3 + 3,
			 (LINES - menH) / 2 + 5,
			 false);
	attroff(COLOR_PAIR(GREY));
	return;
}

void draw_palette_preview(int menW, int menH, int sel, int swap){
	// Drawing the title
	char *title = "All Available Colors";
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(A_UNDERLINE);
	move((LINES - menH) / 2 + 8, (COLS - menW) / 2 + (menW - strlen(title)) / 2);
	printw("%s", title);
	attroff(A_UNDERLINE);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));

	// Drawing the border box, hardcoded for a width of 16 color swatches
	attron(COLOR_PAIR(GREY));
	draw_box((COLS - menW) / 2 + (menW - 48) / 2 - 1,
			 (LINES - menH) / 2 + 9,
			 (COLS + menW) / 2 - (menW - 48) / 2,
			 (LINES + menH) / 2 - 2,
			 true);

	// if all the swatches don't fit, we need scrolling functionality
	int maxScroll = PALSIZE / 16 - (menH - 13) / 2;
	int scrolled = 0;
	while (sel / 16 >= (menH - 13) / 2 + scrolled)
		scrolled++;
	
	// This adds scrolling indicators on the screen
	if (scrolled < maxScroll){
		move((LINES + menH) / 2 - 2, (COLS - menW) / 2 + (menW - 8) / 2);
		printw("vvvvvvvv");
	}
	if (scrolled > 0){
		move((LINES - menH) / 2 + 9, (COLS - menW) / 2 + (menW - 8) / 2);
		printw("^^^^^^^^");
	}
	attroff(COLOR_PAIR(GREY));

	// Drawing the color swatches
	for (int i = 0 + scrolled * 16; i < PALSIZE; i ++){
		if ((i - scrolled * 16) / 16 < (menH - 13) / 2){ // Highly readable, right?
			move((LINES - menH) / 2 + 11 + (i - scrolled * 16) / 16 * 2, (COLS - menW) / 2 + (menW - 48) / 2 + 1 + ((i - scrolled * 16) % 16) * 3);
			attron(COLOR_PAIR(i + 1));		// Y: (LINES - menH) / 2 + 11                + (i - scrolled * 16) / 16 * 2 // 2 is the height of a row
			printw("  ");					// this part is top edge of the window      this part offsets i by the amount of rows scrolled past
			attroff(COLOR_PAIR(i + 1));		//
			attron(COLOR_PAIR(GREY));		// X: (COLS - menW) / 2 + (menW - 48) / 2 + 1       + ((i - scrolled * 16) % 16) * 3 // 3 is the width of a swatch
			printw(" ");					// this part is left edge of the window             this part offsets i by the amount of rows scrolled past
			attroff(COLOR_PAIR(GREY));		//
			if (i == swap){ // Drawing a box around the selected swap color
				attron(COLOR_PAIR(REDLINE));
				draw_box((COLS - menW) / 2 + (menW - 48) / 2 + ((i - scrolled * 16) % 16) * 3,
						 (LINES - menH) / 2 + 10 + (i - scrolled * 16) / 16 * 2,
						 (COLS - menW) / 2 + (menW - 48) / 2 + 3 + ((i - scrolled * 16) % 16) * 3,
						 (LINES - menH) / 2 + 12 + (i - scrolled * 16) / 16 * 2,
						 false);
				attroff(COLOR_PAIR(REDLINE));
			}
			if (i == sel){ // Drawing a box around the selected color
				attron(COLOR_PAIR(GREY));
				draw_box((COLS - menW) / 2 + (menW - 48) / 2 + ((i - scrolled * 16) % 16) * 3,
						 (LINES - menH) / 2 + 10 + (i - scrolled * 16) / 16 * 2,
						 (COLS - menW) / 2 + (menW - 48) / 2 + 3 + ((i - scrolled * 16) % 16) * 3,
						 (LINES - menH) / 2 + 12 + (i - scrolled * 16) / 16 * 2,
						 false);
				attroff(COLOR_PAIR(GREY));
			}
		}
	}
	return;
}

// Colorpicker menu
void colorpicker_menu(unsigned char canvas[], int sel){
	PALETTECOLOR *ptr = PALLIST;
	for (int i = 0; i < sel; i++)
		ptr = ptr->next;
	unsigned char redV = ptr->red, greenV = ptr->green, blueV = ptr->blue;
	int hCol = 0, kInput;
	bool exitSignal = false;
	erase();
	draw_colorpicker_menu(canvas, sel, hCol, redV, greenV, blueV);
	while((kInput = getch()) != KEY_CTRL('X')){
    	switch(kInput){	
            case KEY_LEFT:
			if (hCol > 0)
				hCol--;
			else hCol = 2;
			break;

			case KEY_RIGHT:
			if (hCol < 2)
				hCol++;
			else hCol = 0;
			break;

			case KEY_UP:
			switch (hCol){
				case 0:
				if (redV < 255)
					redV++;
				break;
				case 1:
				if (greenV < 255)
					greenV++;
				break;
				case 2:
				if (blueV < 255)
					blueV++;
				break;
			}
			break;

			case KEY_DOWN:
			switch (hCol){
				case 0:
				if (redV > 0)
					redV--;
				break;
				case 1:
				if (greenV > 0)
					greenV--;
				break;
				case 2:
				if (blueV > 0)
					blueV--;
				break;
			}
			break;	

			case KEY_RESIZE:
			erase();
			draw_colorpicker_menu(canvas, sel, hCol, redV, greenV, blueV);
			break;

			case ' ':
			ptr->red = redV;
			ptr->green = greenV;
			ptr->blue = blueV;
			update_colors();
			UNSAVED = true;
			exitSignal = true;
			break;
		}
		if (exitSignal)
			break;
		erase();
		draw_colorpicker_menu(canvas, sel, hCol, redV, greenV, blueV);
	}
	return;
}

void draw_colorpicker_menu(unsigned char canvas[], int oColor, int sVal, unsigned char redV, unsigned char greenV, unsigned char blueV){
	char *heading = "Color Picker";
	char *controls[] = {"space confirm ",
						"^X Exit       "};
	int menW = 38, menH = 13;
	draw_canvas(canvas);
	draw_borders();
	draw_title();
	draw_heading(heading);
	draw_controls(controls, ARRAY_SIZE(controls));
	if (draw_popup(menW, menH)){
		colorpicker_update(menW, menH, oColor, sVal, redV, greenV, blueV);
	}
	return;
}

void colorpicker_update(int menW, int menH, int oColor, int sVal, unsigned char redV, unsigned char greenV, unsigned char blueV){
	// Drawing the title
	char *title = "Color Editor";
	attron(COLOR_PAIR(GET_THEME(DARKMODE)));
	attron(A_UNDERLINE);
	move((LINES - menH) / 2 + 1, (COLS - menW) / 2 + (menW - strlen(title)) / 2);
	printw("%s", title);
	attroff(A_UNDERLINE);
	attroff(COLOR_PAIR(GET_THEME(DARKMODE)));
	// UI background
	attron(COLOR_PAIR(GREY));
	draw_box((COLS - menW) / 2 + 4,
			 (LINES - menH) / 2 + 2,
			 (COLS + menW) / 2 - 5,
			 (LINES + menH) / 2 - 2,
			 true);
	// RGB buttons
	int selected[3] = {0};
	selected[sVal] = 1;
	char redB[6], greenB[6], blueB[6];
	sprintf(redB, " %i ", redV);
	sprintf(greenB, " %i ", greenV);
	sprintf(blueB, " %i ", blueV);
	move((LINES + menH) / 2 - 4, (COLS - menW) / 2 + 6);
	printw("R:");
	move((LINES + menH) / 2 - 4, (COLS - menW) / 2 + 15);
	printw("G:");
	move((LINES + menH) / 2 - 4, (COLS - menW) / 2 + 24);
	printw("B:");
	attroff(COLOR_PAIR(GREY));
	draw_button(redB, (COLS - menW) / 2 + 9, (LINES + menH) / 2 - 4,  selected[0], 0);
	draw_button(greenB, (COLS - menW) / 2 + 18, (LINES + menH) / 2 - 4,  selected[1], 0);
	draw_button(blueB, (COLS - menW) / 2 + 27, (LINES + menH) / 2 - 4,  selected[2], 0);

	// Swatches
	attron(COLOR_PAIR(oColor + 1));
	for (int i = 0; i < 4; i++){
		move((LINES - menH) / 2 + 4 + i, (COLS - menW) / 2 + 11);
		hline(' ', 8);
	}
	attroff(COLOR_PAIR(oColor + 1));
	init_color(PREVIEW, redV * 3.9, greenV * 3.9, blueV * 3.9);
	init_pair(PREVIEW, WHITE, PREVIEW);
	attron(COLOR_PAIR(PREVIEW));
	for (int i = 0; i < 4; i++){
		move((LINES - menH) / 2 + 4 + i, (COLS + menW) / 2 - 19);
		hline(' ', 8);
	}
	attroff(COLOR_PAIR(PREVIEW));
	return;
}

// Painting
short get_color(int x, int y){
	short color = (mvinch(y, x) & A_COLOR) >> 8;
	return color;
}

void pick_color(unsigned char canvas[]){
	short sSize = ARRAY_SIZE(SELECTOR), selC;
	// We use xy_to_canvas instead of get_color to avoid adding UI colors to the selector
	if (xy_to_canvas(CURX, CURY) > -1)
		selC = canvas[xy_to_canvas(CURX, CURY)] + 1;
	else return;
	bool inSelector = false;
	if (PALSIZE < sSize)
		sSize = PALSIZE;
	for (int i = 0; i < sSize; i++){
		if (SELECTOR[i] == selC){
			SINDEX = i;
			inSelector = true;
			break;
		}
	}
	if (!inSelector)
		SELECTOR[SINDEX] = selC;
	return;
}

int xy_to_canvas(int x, int y){
	int offX = (x - CANVX) / 2;
	int offY = CANVY - y;
	if (offX < 0 || offY < 0 || CANVX + (WIDTH - 1) * 2 < x || CANVY - HEIGHT + 1 > y)
		return -1;
	else
		return (offX + offY * WIDTH);
}

void paint_color(unsigned char canvas[], int x, int y){
	int i = xy_to_canvas(x, y);
	if (i > -1){
		canvas[i] = SELECTOR[SINDEX] - 1;
		UNSAVED = true;
	}
}

void flood_fill(unsigned char canvas[], int x, int y, short oldColor, bool checkArray[]){
	checkArray[xy_to_canvas(x, y)] = true;
	paint_color(canvas, x, y);
	for (int i = -2; i < 3; i += 4){
		if (xy_to_canvas(x + i, y) > -1){
			if (canvas[xy_to_canvas(x + i, y)] + 1 == oldColor && !checkArray[xy_to_canvas(x + i, y)])
				flood_fill(canvas, x + i, y, oldColor, checkArray);
			else checkArray[xy_to_canvas(x + i, y)] = true;
		}
	}
	for (int i = -1; i < 2; i += 2){
		if (xy_to_canvas(x, y + i) > -1){
			if (canvas[xy_to_canvas(x, y + i)] + 1 == oldColor && !checkArray[xy_to_canvas(x, y + i)])
				flood_fill(canvas, x, y + i, oldColor, checkArray);
			else checkArray[xy_to_canvas(x, y + i)] = true;
		}
	}
	return;
}

void paint_bucket(unsigned char canvas[]){
	short oldColor;
	if (xy_to_canvas(CURX, CURY) > -1)
		oldColor = canvas[xy_to_canvas(CURX, CURY)] + 1;
	else return;
	bool checkArray[WIDTH * HEIGHT];
	for (int i = 0; i < WIDTH * HEIGHT; i++)
		checkArray[i] = false;
	flood_fill(canvas, CURX, CURY, oldColor, checkArray);
	return;
}

// Data processing
void load_settings(void){
	// Loads the .cfg file containing the program settings (if it exists)
	// Settings are initialized at default values, so if the file is absent it's all good
	// More settings can be easily added via the switch statement
	FILE *settings = fopen(SETTINGSFILE, "r+");
	if (settings != NULL){
		unsigned char darkmode, autosave;
		char buffer, paletteBuffer[FILENAME_MAX + 1];
		int line = 0;
		while (fread(&buffer, 1, 1, settings)){
			if (buffer == '\n')
				line++;
			if (buffer == ':'){
				switch (line){
					case 1:{
						int letterIndex = 0;
						while (fread(&buffer, 1, 1, settings)){
							if (isprint(buffer) && buffer != '.'){ // Truncate the extension if it was included
								paletteBuffer[letterIndex] = buffer;
								letterIndex++;
							}else{
								paletteBuffer[letterIndex] = '\0';
								if (buffer == '\n')
									line++;
								break;
							}
						}
					}
					break;
					case 2:
					fread(&darkmode, 1, 1, settings);
					break;
					case 3:
					fread(&autosave, 1, 1, settings);
					break;
				}
			}
		}
		fclose(settings);
		sprintf(D_PALETTE, "%s", paletteBuffer);
		darkmode = READ_SETTING(darkmode);
		autosave = READ_SETTING(autosave);
		if (darkmode < 2)
			DARKMODE = darkmode;
		if (autosave < 2)
			AUTOSAVE = autosave;
	}
	return;
}

int load_bitmap(BITMAP *bmp, char *filename){
	// Opens and validates that the chosen bitmap file is compatible with pxshop
	// Returns exit codes for main to return if the file fails a check
	bmp->bmpFile = fopen(filename, "r+");
	if (bmp->bmpFile == NULL){
		printf("Unable to open file: %s\n", filename);
		return 3;
	}
	bmp->info = bmp_get_info(bmp->bmpFile);
	bool failed = false;
	if (bmp->info.Version < 3){
		printf("Only version 3.0+ bitmaps are supported\n");
		fclose(bmp->bmpFile);
		return 4;
	}
	if (bmp->info.Compression != 0){
		printf("Only uncompressed bitmaps are supported\n");
		failed = true;
	}
	if (bmp->info.Bitrate > 8){
		printf("Only bitmaps with indexed colors are supported\n");
		failed = true;
	}
	if (failed){
		fclose(bmp->bmpFile);
		return 4;
	}
	return 0;
}

void load_bmp_palette(BITMAP bmp){
	// Reads the palette info off of our infile and populates the linked list of colors accordingly
	int colors = bmp.info.NumColors;
	fseek(bmp.bmpFile, bmp.info.Offset - sizeof(RGBQUAD) * bmp.info.NumColors, SEEK_SET);
	if (colors > MAXCOLORS)
		colors = MAXCOLORS;
	for (int i = 0; i < colors; i++){
		RGBQUAD buffer;
		fread(&buffer, sizeof(RGBQUAD), 1, bmp.bmpFile);
		PALETTECOLOR *new = malloc(sizeof(PALETTECOLOR));
		new->red = buffer.rgbRed;
		new->green = buffer.rgbGreen;
		new->blue = buffer.rgbBlue;
		new->next = NULL;
		if (PALLIST == NULL)
			PALLIST = new;
		else{
			PALETTECOLOR *ptr = PALLIST;
			while (ptr->next != NULL)
				ptr = ptr->next;
			ptr->next = new;
		}
		PALSIZE++;
	}
	update_bitrate();
	return;
}

int import_palette(char *filename){
	// Reads a .hex file and extracts the palette data from it
	// Format is expected to be ASCII base 16, one palette entry per row, all entries ending with a newline
	// ROW:  [1-2]red  [3-4]green  [5-6]blue  [7-8]EOL
	// Needs to read minimum 2 colors and maximum MAXCOLORS to succeed, which returns the number of colors read
	// Error codes: -2: file not opened -1: empty file 0: too few colors 1: too many colors
	int palColors = 0;
	char buffer[8];
	PALETTECOLOR *lList = NULL;
	FILE *hexFile = fopen(filename, "r");
	if (hexFile == NULL)
		return -2;
	// Read the file line at a time
	while (fread(&buffer, 6, 1, hexFile) == 1){
		unsigned char r, g, b;
		sscanf(buffer, "%02hhx", &r);
		sscanf(buffer + 2, "%02hhx", &g);
		sscanf(buffer + 4, "%02hhx", &b);
		char lineExtra = 0;
		while (lineExtra != '\n')
			if (fread(&lineExtra, 1, 1, hexFile) < 1)
				break;
		PALETTECOLOR *new = malloc(sizeof(PALETTECOLOR));
		new->red = r;
		new->green = g;
		new->blue = b;
		new->next = NULL;
		if (lList == NULL)
			lList = new;
		else{
			for (PALETTECOLOR *ptr = lList; ptr != NULL; ptr = ptr->next){
				if (ptr->next == NULL){
					ptr->next = new;
					break;
				}
			}
		}
		palColors++;
	}
	// Validating the data we read
	if (palColors == 0){
		fclose(hexFile);
		return -1;
	}else if (palColors < 2){
		free_palette_list(lList);
		fclose(hexFile);
		return 0;
	}else if (palColors > MAXCOLORS){
		free_palette_list(lList);
		fclose(hexFile);
		return 1;
	}else{
		if (PALLIST != NULL)
			free_palette_list(PALLIST);
		PALLIST = lList;
		PALSIZE = palColors;
		update_bitrate();
		fclose(hexFile);
		return palColors;
	}
}

void load_default_palette(void){
	// If a new file is being created and no external default palette is set/found, this hardcoded palette is loaded
	unsigned char dpal[] = {0xFF, 0xFF, 0xFF,    0x00, 0x00, 0x00,    0xFF, 0x00, 0x00,    0x00, 0xFF, 0x00,
							0x00, 0x00, 0xFF,    0xFF, 0xFF, 0x00,    0xFF, 0x00, 0xFF,    0x00, 0xFF, 0xFF,
							0xFF, 0x00, 0x80,    0xFF, 0x80, 0x40,    0x80, 0x40, 0x00,    0x00, 0x80, 0x80,
							0x80, 0x00, 0x00,    0x80, 0x00, 0x80,    0x80, 0x80, 0xFF,    0x00, 0xFF, 0x80};
	for (int i = 0; i < sizeof(dpal) / 3; i++){
		PALETTECOLOR *new = malloc(sizeof(PALETTECOLOR));
		new->red = dpal[i * 3];
		new->green = dpal[i * 3 + 1];
		new->blue = dpal[i * 3 + 2];
		new->next = NULL;
		if (PALLIST == NULL)
			PALLIST = new;
		else{
			PALETTECOLOR *ptr = PALLIST;
			while (ptr->next != NULL)
				ptr = ptr->next;
			ptr->next = new;
		}
		PALSIZE++;
	}
	update_bitrate();
	return;
}

void init_new_palette(void){
	int numCols = 0;
	if (D_PALETTE[0] != '\0'){
		char fullpath[FILENAME_MAX + 1];
		sprintf(fullpath, PALETTEDIR"/%s.hex", D_PALETTE);
		numCols = import_palette(fullpath);
	}
	if (numCols < 2)
		load_default_palette();
	return;
}

int generate_pfile_list(PALETTEFILE **fileList){
	// Generates a linked list of the names of .hex files found in the palettes directory
    struct dirent **dirList;
    int numFiles = scandir(PALETTEDIR, &dirList, NULL, alphasort);
	if (numFiles < 1)
		return 0;
	int numPals = 0;
	for (int i = 0; i < numFiles; i++){
		if (check_extension(dirList[i]->d_name, "hex")){
			PALETTEFILE *new = malloc(sizeof(PALETTEFILE));
			new->next = NULL;
			int j = 0;
			while (dirList[i]->d_name[j] != '.'){
				sprintf(&new->name[j], "%c", dirList[i]->d_name[j]);
				j++;
			}
			
			if (*fileList == NULL)
				*fileList = new;
			else{
				PALETTEFILE *ptr = *fileList;
				while (ptr->next != NULL)
					ptr = ptr->next;
				ptr->next = new;
			}
			numPals++;
		}
	}
    for (int i = numFiles; i > 0; i--)
        free(dirList[i - 1]);
    free(dirList);
	return numPals;
}

int count_colors(unsigned char canvas[], int color){
	int counter = 0;
	for (int i = 0; i < WIDTH * HEIGHT; i++){
		if (canvas[i] + 1 == color)
			counter++;
	}
	return counter;
}

int count_pfile_colors(PALETTEFILE *fileList, int index){
	// Counts the number of color entries in a chosen palette file
	// Returns the number of entries
	// First, we get the full filename of our .hex file
	PALETTEFILE *ptr = fileList;
	for (int i = 0; i < index; i++)
		ptr = ptr->next;
	char fileName[FILENAME_MAX + 1];
	sprintf(fileName, PALETTEDIR"/%s.hex", ptr->name);
	// Now we open the file and count the entries
	FILE *hexFile = fopen(fileName, "r");
	if (hexFile == NULL)
		return 0;
	int numCols = 0;
	char buffer[6];
	while (fread(&buffer, 6, 1, hexFile) == 1){
		for (int i = 0; i < 6; i++){
			if (!isalnum(buffer[i])){ // If first 6 chars on each line not alphanumeric, exit
				fclose(hexFile);
				return 0;
			}
		}
		char lineExtra = 0;
		while (lineExtra != '\n') // Skips any whitespace that might be present at EOL
			if (fread(&lineExtra, 1, 1, hexFile) < 1)
				break;
		numCols++;
	}	
	return numCols;
}

// Data updating
void update_bitrate(void){
	// Keeps our global bitrate setting up to date with our color count
	if (PALSIZE < 3)
		BITRATE = 1;
	else if (PALSIZE < 5)
		BITRATE = 2;
	else if (PALSIZE < 17)
		BITRATE = 4;
	else BITRATE = 8;
	return;
}

void update_colors(void){
	// Runs through the linked list and initializes colors and color pairs based on order of appearance
	// The conditional contains a formula that determines if white or black contrasts better against a given color
	// Useful for drawing our cursor etc.
	int i = 0;
	for (PALETTECOLOR *ptr = PALLIST; ptr != NULL; ptr = ptr->next){
		init_color(i, (short) ptr->red * 3.9, (short) ptr->green * 3.9, (short) ptr->blue * 3.9);
		short contrast;
		if (ptr->red * 0.299 + ptr->green * 0.587 + ptr->blue * 0.114 > 127)
			 contrast = BLACK;
		else contrast = WHITE;
		init_pair(i + 1, contrast, i); // Because of the hardcoded pair 0 in ncurses, our pairs are offset by +1
		i++;
	}
	// UI colors have to take up the last few spaces, basically unavoidable?
	init_color(RED, 1000, 0, 0);   		
	init_color(WHITE, 1000, 1000, 1000); 
	init_color(BLACK, 0, 0, 0);		     
	init_color(GREY, 80, 80, 80);	
	init_pair(REDLINE, RED, GREY);	 
	init_pair(RED, WHITE, RED);
	init_pair(WHITE, BLACK, WHITE);
	init_pair(BLACK, WHITE, BLACK);
	init_pair(GREY, WHITE, GREY);
	return;
}

void new_color(int r, int g, int b){
	PALETTECOLOR *new = malloc(sizeof(PALETTECOLOR));
	new->red = r;
	new->green = g;
	new->blue = b;
	new->next = NULL;
	for (PALETTECOLOR *ptr = PALLIST; ptr != NULL; ptr = ptr->next){
		if (ptr->next == NULL){
			ptr->next = new;
			break;
		}
	}
	PALSIZE++;
	update_bitrate();
	UNSAVED = true;
	return;
}

void swap_colors(int colorX, int colorY){
	// Swapping 2 colors - since the structs are simple enough, we'll just trade data
	PALETTECOLOR *ptrX = PALLIST, *ptrY = PALLIST;
	unsigned char rTemp, gTemp, bTemp;
	int i = 0;
	while (i < colorX){
		ptrX = ptrX->next;
		i++;
	}
	i = 0;
	while (i < colorY){
		ptrY = ptrY->next;
		i++;
	}
	rTemp = ptrX->red;
	gTemp = ptrX->green;
	bTemp = ptrX->blue;
	ptrX->red = ptrY->red;
	ptrX->green = ptrY->green;
	ptrX->blue = ptrY->blue;
	ptrY->red = rTemp;
	ptrY->green = gTemp;
	ptrY->blue = bTemp;	
	UNSAVED = true;
	return;
}

void delete_color(unsigned char canvas[], int color){
	// Replace the color with our selected color
	// Shrink all colors with a higher index
	for (int i = 0; i < WIDTH * HEIGHT; i++){
		if (canvas[i] + 1 == color){
			if (canvas[i] != SELECTOR[SINDEX] - 1)
				canvas[i] = SELECTOR[SINDEX] - 1;
			else
				canvas[i] = 0;
		}else if (canvas[i] + 1 > color)
			canvas[i]--;
	}
	// Do the same for our selector
	for (int i = 0; i < 16; i++){
		if (SELECTOR[i] == color)
			SELECTOR[i] = 1;
		else if (SELECTOR[i] > color)
			SELECTOR[i]--;
	}
	// Find our color in the list and delete it
	if (color == 1){
		PALETTECOLOR *temp = PALLIST;
		PALLIST = PALLIST->next;
		free(temp);
	}else{
		PALETTECOLOR *ptr = PALLIST, *prev = PALLIST;
		for (int i = 0; i < color - 1; i++){
			prev = ptr;
			ptr = ptr->next;
		}
		prev->next = ptr->next;
		free(ptr);
	}
	PALSIZE--;
	UNSAVED = true;
	return;
}

// Data writing
int write_settings(void){
	char *fileLines[] = {"[PXSHOP SETTINGS]",
					   "DEFAULT_PALETTE:",
					   "DARKMODE:",
					   "AUTOSAVE:"};
	char darkmode = WRITE_SETTING(DARKMODE);
	char autosave = WRITE_SETTING(AUTOSAVE);
	FILE *settings = fopen(SETTINGSFILE, "w+");
	if (settings == NULL)
		return 0;
	char newLine[] = {0x0D, 0x0A};
	for (int line = 0; line < ARRAY_SIZE(fileLines); line++){
		char buffer;
		for (int j = 0; j < strlen(fileLines[line]); j++){

			buffer = *(fileLines[line] + j);
			fwrite(&buffer, 1, 1, settings);
			if (buffer == ':')
				switch (line){
					case 1:
					fwrite(D_PALETTE, strlen(D_PALETTE), 1, settings);
					break;
					case 2:
					fwrite(&darkmode, 1, 1, settings);
					break;
					case 3:
					fwrite(&autosave, 1, 1, settings);
					break;
				}
		}
		fwrite(&newLine, 2, 1, settings);
	}
	fclose(settings);
	return 1;
}

int write_file(char *filename, unsigned char canvas[]){
	FILE *outfile = fopen(filename, "w");
	if (outfile == NULL)
		return 0;
	BITMAP bmp = {0};
	bmp.bmpFile = outfile;
	bmp.info.Bitrate = (WORD) BITRATE;
	bmp.info.Height = (LONG) HEIGHT;
	bmp.info.Width = (LONG) WIDTH;
	bmp.info.NumColors = (WORD) PALSIZE;
	bmp.info.Offset = (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV3HEADER) + PALSIZE * sizeof(RGBQUAD));
	RGBQUAD outPalette[PALSIZE];
	PALETTECOLOR *ptr = PALLIST;
	for (int i = 0; i < PALSIZE; i++){
		outPalette[i].rgbRed = ptr->red;
		outPalette[i].rgbGreen = ptr->green;
		outPalette[i].rgbBlue = ptr->blue;
		outPalette[i].rgbReserved = 0;
		ptr = ptr->next;
	}
	bmp_from_array(bmp, canvas, outPalette);
	fclose(bmp.bmpFile);
	return 1;
}

int write_palette(char *filename){
	// Writes the current color palette to a hexfile
	// Returns the number of entries written
	char filePath[FILENAME_MAX + 1];
	char *slash = strrchr(filename, '/');
	sprintf(filePath, PALETTEDIR"/%s", slash + 1);
	sprintf(filePath + strlen(filePath) - 3, "hex");
	// Make the directory if it doesn't exist
	if (access(PALETTEDIR, F_OK))
		mkdir(PALETTEDIR, 0777);
	FILE *hexFile = fopen(filePath, "w");
	if (hexFile == NULL)
		return 0;
	int colsWritten = 0;
	PALETTECOLOR *ptr = PALLIST;
	while (ptr != NULL){
		char buffer[8];
		sprintf(buffer, "%02x%02x%02x\n", ptr->red, ptr->green, ptr->blue);
		fwrite(&buffer, 7, 1, hexFile);
		colsWritten++;
		ptr = ptr->next;
	}
	fclose(hexFile);
	return colsWritten;
}

// Misc
int lstring(char *array[], int arraySize){
	// Returns the length of the longest string in a string array (not incl. null byte)
	int ll = 0;
	for (int i = 0; i < arraySize; i++){
		if (strlen(array[i]) > ll)
			ll = strlen(array[i]);
	}
	return ll;
}

void sbfstring(char *buffer, char *string, int maxLength){
	// Truncates the end of a string if it exceeds a certain size
	// Also prints spaces on either end of the new string and a ... if original string is too long
	// Used for button formatting
	sprintf(buffer, " ");
	int i = 0;
	if (strlen(string) + 2 > maxLength){
		while (i < maxLength - 4){
			sprintf(buffer + i + 1, "%c", string[i]);
			i++;
		}
		sprintf(buffer + i, "... ");
	}else{
		while (i < strlen(string)){
			sprintf(buffer + i + 1, "%c", string[i]);
			i++;
		}
		sprintf(buffer + i + 1, " ");
	}
	return;
}

void clamp_positions(void){
	// Clamping the cursor to the window bounds
	if (CURX > COLS - 3)
		CURX = COLS - 3;
	if (CURX < 1)
		CURX = 1;
	if (CURY > LINES - 3)
		CURY = LINES - 3;
	if (CURY < 3)
		CURY = 3;
	if (CURX % 2 == 0)
		CURX--;
	// Clamping the camera to the window bounds
	if (CANVY - HEIGHT > LINES - 4)
		CANVY = LINES - 4 + HEIGHT;
	if (CANVY < 3)
		CANVY = 3;
	if (CANVX + WIDTH * 2 < 3)
		CANVX = 3 - WIDTH * 2;
	if (CANVX > COLS - 3)
		CANVX = COLS - 3;
	if (CANVX % 2 == 0)
		CANVX--;
	return;
}

void free_palette_list(PALETTECOLOR *list){
	// Frees up the memory allocated for a linked list
	PALETTECOLOR *ptr = list;
    while (ptr != NULL){
        PALETTECOLOR *next = ptr->next;
        free(ptr);
        ptr = next;
    }
	return;
}

void free_file_list(PALETTEFILE **list){
	// Frees up the memory allocated for a linked list
	PALETTEFILE *ptr = *list;
    while (ptr != NULL){
        PALETTEFILE *next = ptr->next;
        free(ptr);
        ptr = next;
    }
	return;
}

int check_extension(char *fileName, char * extension){
    char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName || strlen(dot) < 2)
        return 0;
	else if (strcasecmp(dot + 1, extension) == 0)
		return 1;
	else return 0;
}

