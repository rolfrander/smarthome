#include <SmingCore.h>

class Rotary;

/**
 * @brief the RotaryCallback function is called with -1 or 1 as input when rotating, 0 when switch is pushed.
 */
using RotaryCallback = void (*)(Rotary*, int);

enum Direction { UP, DOWN };

/**
 * @brief handling of input from 2 or 3 pin rotary switch with basic debounce.
 */
class Rotary {
private:
    uint8_t pin_a, pin_b, pin_switch;
    uint8_t state = 0;
    uint8_t a, b, sw;
    enum Direction direction;
    int value = 0;
    int min = INT_MIN;
    int max = INT_MAX;
    bool wrapMode = true;
    int errorcounter = 0;
    int switchcounter = 0;
    RotaryCallback callback = NULL;

    uint8_t doPoll();
    void increase();
    void decrease();
    void click();
    void doCallback(int direction);
public:
    Rotary(uint8_t pin_a, uint8_t pin_b);
    Rotary(uint8_t pin_a, uint8_t pin_b, uint8_t pin_switch);
    /**
     * @brief wrapping mode, jumps from max to min and vice versa when overflowing. This is the default mode.
     */
    void setModeWrap(int min, int max, int start);
    /**
     * @brief clamping mode, stops at min or max. Starts at start.
     */
    void setModeClamp(int min, int max, int start);

    void poll();

    void registerCallback(RotaryCallback callback);

    int getValue();

    enum Direction getDirection();
};
