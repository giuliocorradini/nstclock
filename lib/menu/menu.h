#ifndef __menu_common_h
#define __menu_common_h

enum event_cause_t {
    BUTTON_DOWN,
    BUTTON_UP,
    BUTTON_ENTER,
    TOTAL_CAUSES
};

struct menu_event {
    enum event_cause_t cause;
};

#endif