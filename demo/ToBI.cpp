#include "ToBI.h"

// Global characters for console drawing
wchar_t LU = L'\u2554';
wchar_t RU = L'\u2557';
wchar_t LL = L'\u255A';
wchar_t RL = L'\u255D';
wchar_t Vbar = L'\u2551';
wchar_t Hbar = L'\u2550';

// Console utility implementations
void CMD_Init(int width, int height) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD buf = { (SHORT)width, (SHORT)height };
	SetConsoleScreenBufferSize(h, buf);
	SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
	SetConsoleWindowInfo(h, TRUE, &rect);
	CONSOLE_CURSOR_INFO ci;
	if (GetConsoleCursorInfo(h, &ci)) {
		ci.bVisible = false;
		SetConsoleCursorInfo(h, &ci);
	}
}

void gotoxy(short x, short y) {
	COORD coord; coord.X = x; coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void gotoxy(short* x, short* y) {
	COORD coord; coord.X = *x; coord.Y = *y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void WriteSpecialCharAt(short* x, short* y, wchar_t ch) {
	gotoxy(x, y);
	std::cout.flush();
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written = 0;
	WriteConsoleW(h, &ch, 1, &written, NULL);
}

void PrintAt(short* x, short* y, const std::string& s) {
	gotoxy(x, y);
	std::cout << s;
}

void CMD_Color(WORD param1) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), param1);
}

// ---------------------------------------------------------
// Table class implementations
// ---------------------------------------------------------

Table::Table() : V_padding(0), lines(0) {
	x1 = new short(0);
	x2 = new short(0);
	y1 = new short(0);
	y2 = new short(0);
	textAttribute = new WORD(0);
	title = new std::string("");
	contents = new std::vector<std::string>();
}

Table::~Table() {
	delete x1;
	delete x2;
	delete y1;
	delete y2;
	delete textAttribute;
	delete title;
	delete contents;
}

// Deep Copy Constructor
Table::Table(const Table& other) : V_padding(other.V_padding), lines(other.lines) {
	x1 = new short(*other.x1);
	x2 = new short(*other.x2);
	y1 = new short(*other.y1);
	y2 = new short(*other.y2);
	textAttribute = new WORD(*other.textAttribute);
	title = new std::string(*other.title);
	contents = new std::vector<std::string>(*other.contents);
}

// Deep Copy Assignment
Table& Table::operator=(const Table& other) {
	if (this == &other) return *this;
	*x1 = *other.x1;
	*x2 = *other.x2;
	*y1 = *other.y1;
	*y2 = *other.y2;
	*textAttribute = *other.textAttribute;
	*title = *other.title;
	*contents = *other.contents;
	V_padding = other.V_padding;
	lines = other.lines;
	return *this;
}

void Table::setCoords(int a, int b, int c, int d) {
	*x1 = a; *y1 = b; *x2 = c; *y2 = d;
	*textAttribute = 0;
}

void Table::setCoords(short* a, short* b, short* c, short* d) {
	*x1 = *a; *y1 = *b; *x2 = *c; *y2 = *d;
	*textAttribute = 0;
}

void Table::setAttribute(WORD p1) { *textAttribute = p1; }
void Table::setAttribute(WORD p1, WORD p2) { *textAttribute = p1 | p2; }
void Table::setAttribute(WORD p1, WORD p2, WORD p3) { *textAttribute = p1 | p2 | p3; }
void Table::setAttribute(WORD p1, WORD p2, WORD p3, WORD p4) { *textAttribute = p1 | p2 | p3 | p4; }

void Table::setTitle(const std::string& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return;
	std::string line;
	getline(f, line);
	line = ' ';
	getline(f, line);
	*title = line;
}

void Table::setContents(const std::string& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return;
	std::string line;
	int index = 0;
	while (getline(f, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (index > 1 && !line.empty()) {
			contents->push_back(line);
			lines++;
		}
		index++;
	}
}

int Table::parseFile(const std::string& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return -1; // Return -1 on error

	std::string line;
	// Read the first line containing the 10 bits
	if (!getline(f, line)) return -1;

	std::stringstream ss(line);
	int rF, gF, bF, gammaF;  // Foreground
	int rB, gB, bB, gammaB;  // Background
	int t1, t2;              // Type

	// Parse the 10 bits separated by spaces
	if (!(ss >> rF >> gF >> bF >> gammaF >> rB >> gB >> bB >> gammaB >> t1 >> t2)) {
		return -1; // Malformed header
	}

	// Construct the WORD attribute
	WORD attr = 0;
	if (rF) attr |= FOREGROUND_RED;
	if (gF) attr |= FOREGROUND_GREEN;
	if (bF) attr |= FOREGROUND_BLUE;
	if (gammaF) attr |= FOREGROUND_INTENSITY;

	if (rB) attr |= BACKGROUND_RED;
	if (gB) attr |= BACKGROUND_GREEN;
	if (bB) attr |= BACKGROUND_BLUE;
	if (gammaB) attr |= BACKGROUND_INTENSITY;

	*textAttribute = attr;

	// Calculate file type (00 = 0, 01 = 1, 10 = 2, 11 = 3)
	int fileType = (t1 << 1) | t2;

	// Read the remaining lines into contents
	while (getline(f, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (!line.empty()) {
			contents->push_back(line);
			lines++; // Track amount of items for menu navigation
		}
	}

	return fileType;
}

void Table::draw() {
	CMD_Color(*textAttribute);
	WriteSpecialCharAt(x1, y1, LU);
	for (short i = *x1 + 1; i < *x2; i++) { WriteSpecialCharAt(&i, y1, Hbar); }
	WriteSpecialCharAt(x2, y1, RU);
	for (short i = *y1 + 1; i < *y2; i++) { WriteSpecialCharAt(x1, &i, Vbar); WriteSpecialCharAt(x2, &i, Vbar); }
	WriteSpecialCharAt(x1, y2, LL);
	for (short i = *x1 + 1; i < *x2; i++) { WriteSpecialCharAt(&i, y2, Hbar); }
	WriteSpecialCharAt(x2, y2, RL);
}

void Table::drawTitle() {
	short pos;
	CMD_Color(*textAttribute);
	if ((*x2 - *x1) % 2) {
		if ((title->length() / 2) % 2) pos = ((*x2 - *x1) / 2) + (short)(title->length() / 2);
		else pos = ((*x2 + 1 - *x1) / 2) + (short)(title->length() / 2);
	}
	else {
		pos = ((*x2 - *x1) / 2) - (short)(title->length() / 2) + *x1;
	}
	gotoxy(pos, *y1 + 1);
	std::cout << *title;
}

void Table::drawTitle(int V_pad) {
	short pos;
	V_padding = V_pad;
	CMD_Color(*textAttribute);
	if ((*x2 - *x1) % 2) {
		if ((title->length() / 2) % 2) pos = ((*x2 - *x1) / 2) + (short)(title->length() / 2);
		else pos = ((*x2 + 1 - *x1) / 2) + (short)(title->length() / 2);
	}
	else {
		pos = ((*x2 - *x1) / 2) - (short)(title->length() / 2) + *x1;
	}
	gotoxy(pos, *y1 + V_pad);
	std::cout << *title;
}

void Table::drawContents() {
	CMD_Color(*textAttribute);
	for (int i = 0; i < (int)contents->size(); i++) {
		gotoxy(*x1 + 2, *y1 + 3 + 2 * i);
		std::cout << (*contents)[i] << std::endl;
	}
}

void Table::drawLine(int line, bool highlight) {
	if (line < 0 || line >= contents->size()) return;
	CMD_Color(*textAttribute);
	if (highlight) CMD_Color((*textAttribute << 4) & 0x00FF);
	gotoxy(*x1 + 2, *y1 + 3 + line * 2);
	std::cout << (*contents)[line];
}

void Table::drawAll() {
	draw(); drawTitle(); drawContents();
}

void Table::clear() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	WriteSpecialCharAt(x1, y1, L' ');
	for (short i = *x1 + 1; i < *x2; i++) { WriteSpecialCharAt(&i, y1, L' '); }
	WriteSpecialCharAt(x2, y1, L' ');
	for (short i = *y1 + 1; i < *y2; i++) { WriteSpecialCharAt(x1, &i, L' '); WriteSpecialCharAt(x2, &i, L' '); }
	WriteSpecialCharAt(x1, y2, L' ');
	for (short i = *x1 + 1; i < *x2; i++) { WriteSpecialCharAt(&i, y2, L' '); }
	WriteSpecialCharAt(x2, y2, L' ');
}

void Table::clearTitle() {
	short pos;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	if ((*x2 - *x1) % 2) {
		if ((title->length() / 2) % 2) pos = ((*x2 - *x1) / 2) + (short)(title->length() / 2);
		else pos = ((*x2 + 1 - *x1) / 2) + (short)(title->length() / 2);
	}
	else {
		pos = ((*x2 - *x1) / 2) - (short)(title->length() / 2) + *x1;
	}
	gotoxy(pos, *y1 + 1);
	for (int i = 0; i < (int)title->length(); i++) std::cout << ' ';
}

void Table::clearTitle(int V_pad) {
	short pos;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	if ((*x2 - *x1) % 2) {
		if ((title->length() / 2) % 2) pos = ((*x2 - *x1) / 2) + (short)(title->length() / 2);
		else pos = ((*x2 + 1 - *x1) / 2) + (short)(title->length() / 2);
	}
	else {
		pos = ((*x2 - *x1) / 2) - (short)(title->length() / 2) + *x1;
	}
	gotoxy(pos, *y1 + V_pad);
	for (int i = 0; i < (int)title->length(); i++) std::cout << ' ';
}

void Table::clearContents() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	for (int i = 0; i < (int)contents->size(); i++) {
		gotoxy(*x1 + 2, *y1 + 3 + i * 2);
		for (int j = 0; j < (int)(*contents)[i].length(); j++) std::cout << ' ';
	}
}

void Table::clearLine(int line) {
	if (line < 0 || line >= contents->size()) return;
	CMD_Color(*textAttribute);
	gotoxy(*x1 + 2, *y1 + 3 + line * 2);
	for (int i = 0; i < (int)(*contents)[line].length(); i++) std::cout << ' ';
}

void Table::clearAll() {
	clear(); clearContents();
	if (V_padding) clearTitle(V_padding);
	else clearTitle();
}

// ---------------------------------------------------------
// Logo class implementations
// ---------------------------------------------------------

Logo::Logo() : width(0), height(0), x(0), y(0) {
	textAttribute = new WORD(0);
	lines = new std::vector<std::string>();
}

Logo::~Logo() {
	delete textAttribute;
	delete lines;
}

// Deep Copy Constructor
Logo::Logo(const Logo& other) : width(other.width), height(other.height), x(other.x), y(other.y) {
	textAttribute = new WORD(*other.textAttribute);
	lines = new std::vector<std::string>(*other.lines);
}

// Deep Copy Assignment
Logo& Logo::operator=(const Logo& other) {
	if (this == &other) return *this;
	width = other.width;
	height = other.height;
	x = other.x;
	y = other.y;
	*textAttribute = *other.textAttribute;
	*lines = *other.lines;
	return *this;
}

void Logo::setAttribute(WORD p1) { *textAttribute = p1; }
void Logo::setAttribute(WORD p1, WORD p2) { *textAttribute = p1 | p2; }
void Logo::setAttribute(WORD p1, WORD p2, WORD p3) { *textAttribute = p1 | p2 | p3; }
void Logo::setAttribute(WORD p1, WORD p2, WORD p3, WORD p4) { *textAttribute = p1 | p2 | p3 | p4; }

void Logo::setCoords(short a, short b) { x = a; y = b; }

void Logo::loadLogo(const std::string& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return;
	std::string line;
	getline(f, line);
	line = ' ';
	while (getline(f, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		lines->push_back(line);
		if ((int)line.size() > width) width = (int)line.size();
		height++;
	}
}

void Logo::draw() {
	CMD_Color(*textAttribute);
	for (int rows = 0; rows < height; rows++) {
		gotoxy(x, y + rows);
		std::cout << (*lines)[rows];
	}
}

void Logo::clear() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	for (int i = 0; i < height; i++) {
		gotoxy(x, y + i);
		for (int j = 0; j < width; j++) std::cout << ' ';
	}
}

int Logo::parseFile(const std::string& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return -1; // Return -1 on error

	std::string line;
	// Read the first line containing the 10 bits
	if (!getline(f, line)) return -1;

	std::stringstream ss(line);
	int rF, gF, bF, gammaF;  // Foreground
	int rB, gB, bB, gammaB;  // Background
	int t1, t2;              // Type

	// Parse the 10 bits separated by spaces
	if (!(ss >> rF >> gF >> bF >> gammaF >> rB >> gB >> bB >> gammaB >> t1 >> t2)) {
		return -1; // Malformed header
	}

	// Construct the WORD attribute
	WORD attr = 0;
	if (rF) attr |= FOREGROUND_RED;
	if (gF) attr |= FOREGROUND_GREEN;
	if (bF) attr |= FOREGROUND_BLUE;
	if (gammaF) attr |= FOREGROUND_INTENSITY;

	if (rB) attr |= BACKGROUND_RED;
	if (gB) attr |= BACKGROUND_GREEN;
	if (bB) attr |= BACKGROUND_BLUE;
	if (gammaB) attr |= BACKGROUND_INTENSITY;

	*textAttribute = attr;

	// Calculate file type (00 = 0, 01 = 1, 10 = 2, 11 = 3)
	int fileType = (t1 << 1) | t2;

	// Read the remaining art into the logo lines
	while (getline(f, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		lines->push_back(line);
		if ((int)line.size() > width) width = (int)line.size();
		height++;
	}

	return fileType;
}

// ---------------------------------------------------------
// Submenu class implementations
// ---------------------------------------------------------

Submenu::Submenu() : Table(), currentLine(0), prevLine(0) {}

Submenu::Submenu(const Submenu& other) : Table(other), currentLine(other.currentLine), prevLine(other.prevLine) {}

Submenu& Submenu::operator=(const Submenu& other) {
	if (this == &other) return *this;
	Table::operator=(other); // Call base class assignment
	currentLine = other.currentLine;
	prevLine = other.prevLine;
	return *this;
}

void Submenu::setCurrentLine(int line) { currentLine = line; }
void Submenu::setPrevLine(int line) { prevLine = line; }