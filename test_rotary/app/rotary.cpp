#include "rotary.h"

// statically allocate 16 callback datastructures
#define ALLOC_CALLBACK_REF 16
#define ERROR_LIMIT 10
#define SWITCH_TIMEOUT 1000

struct callback_reference {
    Rotary* r;
    RotaryCallback c;
    int direction;
    int taken;
} allocated_ref[ALLOC_CALLBACK_REF];

callback_reference* alloc_ref() {
    for(int i=0; i<ALLOC_CALLBACK_REF; i++) {
        if(!allocated_ref[i].taken) {
            allocated_ref[i].taken = true;
            return &(allocated_ref[i]);
        }
    }
    return NULL;
}

void free_ref(callback_reference* ref) {
    // this is probably very unsafe, should check that the reference is within the bounds
    // of the preallocated array...
    ref->taken = false;
}

void trampoline(void* r) {
    callback_reference* ref = (callback_reference*)r;
    ref->c(ref->r, ref->direction);
    free_ref(ref);
}

callback_reference* get_ref(Rotary* r, RotaryCallback c, int direction) {
    callback_reference* ref = alloc_ref();
    if(ref != NULL) {
        ref->r = r;
        ref->c = c;
        ref->direction = direction;
    }
    return ref;
}

uint8_t Rotary::doPoll() {
    // using a pullup resistor, som inverting all reads to get the intuitive values
    a = !digitalRead(pin_a);
    b = !digitalRead(pin_b);
    if(pin_switch >= 0) {
        sw = !digitalRead(pin_switch);
    }
    return (a<<1) | b;
}

void Rotary::doCallback(int direction) {
    if(this->callback) {
        callback_reference* ref = get_ref(this, this->callback, direction);
        if(ref != NULL) {
            // TODO: some error-handling
            System.queueCallback(trampoline, ref);
        } else {
            Serial.println("rotary could not allocate reference object for callback!");
        }
    }
}

void Rotary::increase() {
    if(value == max) {
        if(wrapMode) {
            value = min;
        } else {
            value = max;
        }
    } else {
        value++;
    }
    doCallback(1);
}

void Rotary::decrease() {
    if(value == min) {
        if(wrapMode) {
            value = max;
        } else {
            value = min;
        }
    } else {
        value--;
    }
    doCallback(-1);
}

void Rotary::click() {
    doCallback(0);
}

Rotary::Rotary(uint8_t pin_a, uint8_t pin_b) {
    Rotary(pin_a, pin_b, -1);
}

Rotary::Rotary(uint8_t pin_a, uint8_t pin_b, uint8_t pin_switch) {
    this->pin_a = pin_a;
    this->pin_b = pin_b;
    this->pin_switch = pin_switch;

    pinMode(pin_a, INPUT_PULLUP);
    pinMode(pin_b, INPUT_PULLUP);
    if(pin_switch >= 0) {
        pinMode(pin_switch, INPUT_PULLUP);
    }

    this->state = doPoll();
}

/**
 * @brief wrapping mode, jumps from max to min and vice versa when overflowing. This is the default mode.
 */
void Rotary::setModeWrap(int min, int max, int start) {
    this->min = min;
    this->max = max;
    this->value = start;
    this->wrapMode = true;
}

/**
 * @brief clamping mode, stops at min or max. Starts at start.
 */
void Rotary::setModeClamp(int min, int max, int start) {
    setModeWrap(min, max, start);
    this->wrapMode = false;
}

void Rotary::poll() {
    /*        __:__:  :
       B  ___|  :  |__:____
             :  :__:__:
       A  ___:__|  :  |____
    state  0  1  3  2  0
    */
    uint8_t nextState = doPoll();

    if(this->sw && (this->switchcounter == 0)) {
        this->click();
        this->switchcounter = SWITCH_TIMEOUT;
    }

    if(this->switchcounter > 0) {
        this->switchcounter--;
    }

    switch(this->state) {
        case 0:
            if(nextState == 1) {
                this->increase();
                this->direction = UP;
                this->state = nextState;
            } else if(nextState == 2) {
                this->decrease();
                this->direction = DOWN;
                this->state = nextState;
            }
            break;
        case 1:
            if((nextState == 3 && this->direction == UP) ||
               (nextState == 0 && this->direction == DOWN)) {
                this->state = nextState;
            }
            break;
        case 3:
            if((nextState == 2 && this->direction == UP) ||
               (nextState == 1 && this->direction == DOWN)) {
                this->state = nextState;
            }
            break;
        case 2:
            if((nextState == 0 && this->direction == UP) ||
               (nextState == 3 && this->direction == DOWN)) {
                this->state = nextState;
            }
            break;
    }
    if(this->state != nextState) {
        if(nextState == 0) {
            // sometimes the switch returns to state 0 without
            // the state following, if this happens some times in a row
            // assume we are back to neutral and reset the state
            if(this->errorcounter > ERROR_LIMIT) {
                this->state = 0;
                this->errorcounter = 0;
            } else {
                this->errorcounter++;
            }
        }
    } else {
        this->errorcounter = 0;
    }
}

/**
 * @brief currently only one callback is supported
 */
void Rotary::registerCallback(RotaryCallback callback) {
    this->callback = callback;
}

int Rotary::getValue() {
    return this->value;
}

enum Direction Rotary::getDirection() {
    return this->direction;
}