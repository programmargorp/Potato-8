#ifndef GFX_H_
#define GFX_H_

#include <iostream>
#include <cstdint>
#include "build/emu.h"

enum gfx_status{
    GFX_OK
};

class gfx{
    private:
        uint8_t _frame_buffer[CHIP8_HORIZONTAL * CHIP8_VERTICAL];
    public:
        gfx();
        gfx_status clear_framebuffer();
        gfx_status draw(uint8_t vx, uint8_t vy, uint8_t *buf, uint8_t n, uint8_t *_reg);
        gfx_status get_framebuffer(uint8_t *buf);
};


#endif
