#include <iostream>
#include "gfx.h"
#include "build/emu.h"

gfx::gfx(){
    if(this->clear_framebuffer() != GFX_OK){
        printf("[ERR] COULD NOT CLEAR FRAMEBUFFER AT GFX CREATION");
    }
}

gfx_status gfx::clear_framebuffer(){
    for(auto x=0; x<(CHIP8_HORIZONTAL * CHIP8_VERTICAL); ++x){
        _frame_buffer[x] = 0;
    }
    return GFX_OK;
}

gfx_status gfx::draw(uint8_t vx, uint8_t vy, uint8_t *buf, uint8_t n, uint8_t *_reg){
    _reg[0xF] = 0;
    for(auto y = vy; y < (vy + n); ++y){
        uint8_t b = buf[y - vy];
        for(auto x = vx; x < (vx + 8); ++x){
            uint8_t t = (b >> (7 - (x - vx))) & 0x01;
            uint8_t dx, dy;
            dx = x % 64;
            dy = y % 32;
            if(_frame_buffer[dx + (dy * 64)] == 1 && t == 1) _reg[0xF] = 1;
            _frame_buffer[dx + (dy * 64)] ^= t;
        }
    }
    return GFX_OK;
}

gfx_status gfx::get_framebuffer(uint8_t *buf){
    for(auto x=0; x<(CHIP8_HORIZONTAL * CHIP8_VERTICAL); ++x){
        buf[x] = _frame_buffer[x];
    }
    return GFX_OK;
}