#ifndef MEM_H_
#define MEM_H_

#include <iostream>
#include <cstdint>
#include "build/emu.h"

enum mem_status{
    MEM_OK,
    MEM_ERR_OUT_OF_BOUNDS
};

class mem{
    private:
        uint8_t _mem[CHIP8_MEM_SIZE];
    public:
        mem();
        mem_status write_mem(uint8_t *buf, uint16_t loc, uint16_t size);
        mem_status read_mem(uint8_t *buf, uint16_t loc, uint16_t size);
};


#endif