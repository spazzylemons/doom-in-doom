enum evtype_t {
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_buttondown,
    ev_buttonup,
    ev_quit
};

enum ccmd_t {
    CCMD_ATTACK,
    CCMD_USE,
    CCMD_FORWARD,
    CCMD_BACK,
    CCMD_MOVELEFT,
    CCMD_MOVERIGHT,
    CCMD_LEFT,
    CCMD_RIGHT,
    CCMD_SPEED,
    CCMD_STRAFE,

    CCMD_WEAPNEXT,
    CCMD_WEAPPREV,

    CCMD_SLOT_1,
    CCMD_SLOT_2,
    CCMD_SLOT_3,
    CCMD_SLOT_4,
    CCMD_SLOT_5,
    CCMD_SLOT_6,
    CCMD_SLOT_7,
    CCMD_SLOT_8,

    CCMD_TOGGLEMAP,
    CCMD_AM_PANLEFT,
    CCMD_AM_PANRIGHT,
    CCMD_AM_PANUP,
    CCMD_AM_PANDOWN,
    CCMD_AM_ZOOMIN,
    CCMD_AM_ZOOMOUT,
    CCMD_AM_GOBIG,
    CCMD_AM_TOGGLEFOLLOW,
    CCMD_AM_TOGGLEGRID,
    CCMD_AM_SETMARK,
    CCMD_AM_CLEARMARKS,

    CCMD_SPYNEXT,
    CCMD_SIZEUP,
    CCMD_SIZEDOWN,

    CCMD_MENU_MAIN,
    CCMD_MENU_HELP,
    CCMD_MENU_SAVE,
    CCMD_MENU_LOAD,
    CCMD_MENU_OPTIONS,
    CCMD_QUICKSAVE,
    CCMD_ENDGAME,
    CCMD_TOGGLEMESSAGES,
    CCMD_QUICKLOAD,
    CCMD_MENU_QUIT,
    CCMD_BUMPGAMMA
};

class event_t {
    evtype_t type;
    int data1, data2, data3;
}
