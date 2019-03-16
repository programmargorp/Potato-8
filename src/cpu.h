#ifndef CPU_H_
#define CPU_H_

#include <iostream>
#include <string.h>
#include <cstdint>
#include "mem.h"
#include "gfx.h"
#include "build/emu.h"

enum cpu_status{
    CPU_OK,
    CPU_ROM_READ_FAILED,
    CPU_ROM_FILE_TOO_LARGE,
    CPU_ROM_FILE_FAILED,
    CPU_GFX_ERR,
    CPU_SP_OUT_OF_BOUNDS,
    CPU_PC_OUT_OF_BOUNDS,
    CPU_MEM_ERR,
    CPU_INVALID_INSTRUCTION,
    CPU_DECODE_FAILED
};

struct instruction{
    uint16_t i;
    uint16_t nnn;
    uint8_t n;
    uint8_t x;
    uint8_t y;
    uint8_t kk;
};

class cpu{
    private:
        uint8_t _reg[16];
        uint16_t _I;
        uint8_t _DT;
        uint8_t _ST;
        uint16_t _pc;
        uint8_t _sp;
        uint16_t _stack[CHIP8_STACK_SIZE];
        uint8_t _key[16];

        uint8_t _draw;

        uint64_t _last_loop_time;

        struct instruction _inst;

        mem *_m;
        gfx *_g;

        cpu_status cls();
        cpu_status ret();
        cpu_status jp();
        cpu_status call();
        cpu_status se_vx_byte();
        cpu_status sne_vx_byte();
        cpu_status se_vx_vy();
        cpu_status ld_vx_byte();
        cpu_status add_vx_byte();
        cpu_status ld_vx_vy();
        cpu_status or_vx_vy();
        cpu_status and_vx_vy();
        cpu_status xor_vx_vy();
        cpu_status add_vx_vy();
        cpu_status sub_vx_vy();
        cpu_status shr_vx();
        cpu_status subn_vx_vy();
        cpu_status shl_vx();
        cpu_status sne_vx_vy();
        cpu_status ld_i_addr();
        cpu_status jp_v0_addr();
        cpu_status rnd_vx_byte();
        cpu_status drw_vx_vy_n();
        cpu_status skp_vx();
        cpu_status sknp_vx();
        cpu_status ld_vx_dt();
        cpu_status ld_vx_k();
        cpu_status ld_dt_vx();
        cpu_status ld_st_vx();
        cpu_status add_i_vx();
        cpu_status ld_f_vx();
        cpu_status ld_b_vx();
        cpu_status ld_i_vx();
        cpu_status ld_vx_i();

        void print_cpu_state();

    public:
        cpu();
        mem* get_mem_object();
        gfx* get_gfx_object();
        cpu_status load_binary(char *file);
        cpu_status execute_instruction();
        cpu_status get_framebuffer(uint8_t *buf);
        void set_key(uint8_t key);
        void unset_key(uint8_t key);
        uint8_t get_draw();

};

#endif