#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/Adafruit_GFX.h>
#include "grid.h"

const inline int maxInt(int a, int b) {
	return (a>b) ? a : b;
}

const inline int minInt(int a, int b) {
	return (a<b) ? a : b;
}

void Grid::drawCell(int col, int row, String txt, int foreground, int background) {
    this->gfx->setTextSize(this->textSize);
    this->gfx->setTextColor(foreground);
    int x = this->margin;
    if(col > 0) { x += firstColWith; }
    if(col > 1) { x += colWith * (col-1); }
    int y = this->margin + row * textSize * 8;
    int width = (col == 0 ? firstColWith : colWith) * textSize * 6;
    int height = textSize * 8;
    this->gfx->fillRect(x, y, width, height, background);
    this->gfx->setCursor(x, y);
    this->gfx->print(txt);
}

Grid::Grid(Adafruit_GFX *_gfx, int margin) {
    this->gfx = _gfx;
    this->margin = margin;
}

void Grid::init(String leftCol[], String topRow[]) {
    this->gfx->fillScreen(ILI9341_BLACK);

    int maxLeftCol = 0;
    int rows = 0;
    for(int i=0; leftCol[i]; i++) {
        maxLeftCol = maxInt(maxLeftCol, leftCol[i].length()+1);
        rows++;
    }
    int maxTopRow = 0;
    int cols = 0; // this does not include leftmost column
    for(int i=0; topRow[i]; i++) {
        maxTopRow = maxInt(maxTopRow, topRow[i].length()+1);
        cols++;
    }
    int maxCharWith = (gfx->width() - this->margin*2) / (maxLeftCol + maxTopRow*cols);
    int maxCharHeight = (gfx->height() - this->margin*2) / rows;
    this->textSize = minInt(maxCharWith / 6, maxCharHeight / 8);

    this->firstColWith = maxLeftCol * this->textSize * 6;
    this->colWith = maxTopRow * this->textSize * 6;

    for(int i=0; i<rows; i++) {
        this->drawCell(0, i, leftCol[i], ILI9341_WHITE, ILI9341_DARKGREY);
    }
    for(int i=0; i<cols; i++) {
        this->drawCell(i+1, 0, topRow[i], ILI9341_YELLOW, ILI9341_BLUE);
    }
}

void Grid::updatecell(int col, int row, String text) {
    this->drawCell(col+1, row+1, text, ILI9341_GREEN, ILI9341_BLACK);
}
