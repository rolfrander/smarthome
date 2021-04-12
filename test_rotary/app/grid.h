class Grid {
private:
    int margin;
    int firstColWith;
    int colWith;
    int textSize;
    Adafruit_GFX *gfx;

    void drawCell(int col, int row, String txt, int foreground, int background);
    
public:
    Grid(Adafruit_GFX *_gfx, int margin);
    void init(String leftCol[], String topRow[]);
    void updatecell(int col, int row, String text);
};

