#include "ToBI.h"
#include <conio.h>
#include <vector>
#include <string>

using namespace std;

int currentSelected = 0;
int prevSelected = 1;
bool inSubmenu = false;

int main() {
	CMD_Init(190, 40);

	Table options;
	options.setCoords(64, 3, 128, 35);
	options.parseFile("options.txt");
	//options.setContents("options.txt");
	options.setAttribute(FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_INTENSITY);
	options.drawAll();
	options.drawLine(currentSelected, true);

	Table details;
	details.setCoords(3, 13, 61, 35);
	details.parseFile("details.txt");
	//details.setContents("details.txt");
	details.setAttribute(FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_INTENSITY);
	details.drawAll();

	// Because of the Rule of Three implementations, this vector creation is now entirely safe
	// and will dynamically allocate and duplicate memory without creating dangling pointers.
	vector<Submenu> submenu(options.lines);
	for (int i = 0; i < options.lines; i++) {
		submenu[i].setCoords(64, 3, 128, 35);
		submenu[i].setTitle("submenu" + to_string(i) + ".txt");
		submenu[i].setContents("submenu" + to_string(i) + ".txt");
		submenu[i].setAttribute(FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_INTENSITY);
	}

	Logo MeTR;
	MeTR.setAttribute(FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_INTENSITY);
	MeTR.parseFile("MeTR.txt");
	MeTR.setCoords(6, 3);
	MeTR.draw();

	while (true) {
		if (_kbhit()) {
			unsigned char ch = _getch();
			if (ch == 224) {
				ch = _getch();
				if (ch == 72) {  // Up arrow
					if (currentSelected > 0) { // Fixed clamping boundary logic
						prevSelected = currentSelected;
						currentSelected--;
						options.clearLine(prevSelected);
						options.drawLine(prevSelected, false);
						options.drawLine(currentSelected, true);
					}
				}
				if (ch == 80) {  // Down arrow
					if (currentSelected < options.lines - 1) { // Fixed clamping boundary logic
						prevSelected = currentSelected;
						currentSelected++;
						options.clearLine(prevSelected);
						options.drawLine(prevSelected, false);
						options.drawLine(currentSelected, true);
					}
				}
			}
			else if (ch == 27) {  // ESC
				if (!inSubmenu) break;
				else {
					inSubmenu = false;
					submenu[currentSelected].clearContents();
					submenu[currentSelected].clearTitle();
					options.drawAll();
					options.drawLine(currentSelected, true);
				}
			}
			else if (ch == 13) {  // Enter
				if (!inSubmenu && options.lines > 0) {
					options.clearContents();
					options.clearTitle();
					submenu[currentSelected].drawContents();
					submenu[currentSelected].drawTitle();
					inSubmenu = true;
					submenu[currentSelected].setCurrentLine(0);
					submenu[currentSelected].setPrevLine(1);
				}
			}
		}
	}

	options.clearAll();
	details.clearAll();
	MeTR.clear();
	CMD_Color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	return 0;
}