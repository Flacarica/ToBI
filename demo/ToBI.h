#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Console utility functions
void CMD_Init(int width, int height);
void gotoxy(short x, short y);
void gotoxy(short* x, short* y);
void WriteSpecialCharAt(short* x, short* y, wchar_t ch);
void PrintAt(short* x, short* y, const std::string& s);
void CMD_Color(WORD param1);

// Base drawable interface
class Drawable {
public:
	virtual void draw() = 0;
	virtual void clear() = 0;
	virtual int parseFile(const std::string& filename) = 0; // Changed to return int
	virtual ~Drawable() = default;
};

// Table class for displaying boxed content
class Table : public Drawable {
protected:
	short* x1;
	short* x2;
	short* y1;
	short* y2;
	int V_padding;
	WORD* textAttribute;
	std::string* title;
	std::vector<std::string>* contents;

public:
	int lines;

	Table();
	~Table() override;

	Table(const Table& other);
	Table& operator=(const Table& other);

	void setCoords(int a, int b, int c, int d);
	void setCoords(short* a, short* b, short* c, short* d);

	void setAttribute(WORD p1);
	void setAttribute(WORD p1, WORD p2);
	void setAttribute(WORD p1, WORD p2, WORD p3);
	void setAttribute(WORD p1, WORD p2, WORD p3, WORD p4);

	void setTitle(const std::string& filename);
	void setContents(const std::string& filename);
	int parseFile(const std::string& filename) override; // Changed to return int

	void draw() override;
	void drawTitle();
	void drawTitle(int V_pad);
	void drawContents();
	void drawLine(int line, bool highlight);
	void drawAll();

	void clear() override;
	void clearTitle();
	void clearTitle(int V_pad);
	void clearContents();
	void clearLine(int line);
	void clearAll();
};

// Logo class for displaying ASCII art
class Logo : public Drawable {
private:
	int width;
	int height;
	short x;
	short y;
	WORD* textAttribute;
	std::vector<std::string>* lines;

public:
	Logo();
	~Logo() override;

	Logo(const Logo& other);
	Logo& operator=(const Logo& other);

	void setAttribute(WORD p1);
	void setAttribute(WORD p1, WORD p2);
	void setAttribute(WORD p1, WORD p2, WORD p3);
	void setAttribute(WORD p1, WORD p2, WORD p3, WORD p4);

	void setCoords(short a, short b);
	void loadLogo(const std::string& filename);

	void draw() override;
	void clear() override;
	int parseFile(const std::string& filename) override; // Changed to return int
};

// Submenu class for handling menu items
class Submenu : public Table {
private:
	int currentLine;
	int prevLine;

public:
	Submenu();
	~Submenu() override = default;

	Submenu(const Submenu& other);
	Submenu& operator=(const Submenu& other);

	void setCurrentLine(int line);
	void setPrevLine(int line);
};