#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include "cpu.h"
#include "mem.h"
#include "gfx.h"
#include "build/emu.h"
#include <chrono>

void log(std::string s){
    printf("[LOG] %s\n", s.c_str());
}

void warn(std::string s){
    printf("[WAR] %s\n", s.c_str());
}

void err(std::string s){
    printf("[ERR] %s\n", s.c_str());
}

cpu::cpu(){
    _sp = 0;
    _pc = CHIP8_PROG_START_ADDR;
    _DT = 0;
    _ST = 0;
    _draw = 0;

    _m = new mem();
    _g = new gfx();

    _last_loop_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Seed RNG
    srand(time(NULL));
}

void cpu::set_key(uint8_t key){
    _key[key] = 1;
}

void cpu::unset_key(uint8_t key){
    _key[key] = 0;
}

uint8_t cpu::get_draw(){
    uint8_t t = _draw;
    return t;
}

cpu_status cpu::get_framebuffer(uint8_t *buf){
    _g->get_framebuffer(buf);
    _draw = 0;
    return CPU_OK;
}

cpu_status cpu::load_binary(char *file){
    std::ifstream rom;
    rom.open(file, std::ios::binary);
    if(rom){
        rom.seekg(0, std::ios::end);
        int size = rom.tellg();

        if(size <= CHIP8_ROM_SIZE){
            log("Loading ROM");
            rom.seekg(0, std::ios::beg);
            uint8_t buf[size];
            rom.read((char *) buf, size);

            if(_m->write_mem(buf, 0x200, size) == MEM_OK){
                return CPU_OK;
            }
            return CPU_ROM_READ_FAILED;
        }else{
            return CPU_ROM_FILE_TOO_LARGE;
        }
    }else{
        return CPU_ROM_FILE_FAILED;
    }
}

mem* cpu::get_mem_object(){
    return _m;
}

gfx* cpu::get_gfx_object(){
    return _g;
}

void cpu::print_cpu_state(){
    printf("PC: %3x SP: %3x I: %3x DT: %2x ST: %2x DRW: %1x INST: %4x\n", _pc, _sp, _I, _DT, _ST, _draw, _inst.i);
    printf("Reg: ");
    for(auto x=0; x<16; ++x)
        printf("%2x ", _reg[x]);
    printf("\nStack: ");
    for(auto x=0; x<16; ++x)
        printf("%4x ", _stack[x]);
    printf("\n\n");
}

cpu_status cpu::execute_instruction(){
    if(DEBUG_ENABLED) print_cpu_state();

    uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if(now - _last_loop_time > 16667){
        _DT -= (_DT > 0) ? 1 : 0;
        _ST -= (_ST > 0) ? 1 : 0;
        _last_loop_time = now;
    }
    // Fetch instruction
    uint8_t buf[2];
    _m->read_mem(buf, _pc, 2);
    _inst.i = (buf[0] << 8) | buf[1];
    _inst.nnn = _inst.i & 0x0FFF;
    _inst.n = _inst.i & 0x000F;
    _inst.x = (_inst.i & 0x0F00) >> 8;
    _inst.y = (_inst.i & 0x00F0) >> 4;
    _inst.kk = (_inst.i & 0x00FF);

    _pc += 2;

    // Decode instruction
    switch(_inst.i >> 12){
        case 0x0:
            switch(_inst.i & 0x00FF){
                case 0xE0: return cls(); break;
                case 0xEE: return ret(); break;
                default:
                    err("INVALID INSTRUCTION");
                    return CPU_INVALID_INSTRUCTION;
                    break;
            }
            break;
        case 0x1: return jp(); break;
        case 0x2: return call(); break;
        case 0x3: return se_vx_byte(); break;
        case 0x4: return sne_vx_byte(); break;
        case 0x5: return se_vx_vy(); break;
        case 0x6: return ld_vx_byte(); break;
        case 0x7: return add_vx_byte(); break;
        case 0x8: 
            switch(_inst.i & 0x000F){
                case 0x0: return ld_vx_vy(); break;
                case 0x1: return or_vx_vy(); break;
                case 0x2: return and_vx_vy(); break;
                case 0x3: return xor_vx_vy(); break;
                case 0x4: return add_vx_vy(); break;
                case 0x5: return sub_vx_vy(); break;
                case 0x6: return shr_vx(); break;
                case 0x7: return subn_vx_vy(); break;
                case 0xE: return shl_vx(); break;
                default:
                    err("INVALID INSTRUCTION");
                    return CPU_INVALID_INSTRUCTION;
                    break;
            }
            break;
        case 0x9: return sne_vx_vy(); break;
        case 0xA: return ld_i_addr(); break;
        case 0xB: return jp_v0_addr(); break;
        case 0xC: return rnd_vx_byte(); break;
        case 0xD: return drw_vx_vy_n(); break;
        case 0xE: 
            switch(_inst.i & 0xFF){
                case 0x9E: return skp_vx(); break;
                case 0xA1: return sknp_vx(); break;
                default: 
                    err("INVALID INSTRUCTION");
                    return CPU_INVALID_INSTRUCTION;
                    break;
            }
            break;
        case 0xF:
            switch(_inst.i & 0xFF){
                case 0x07: return ld_vx_dt(); break;
                case 0x0A: return ld_vx_k(); break;
                case 0x15: return ld_dt_vx(); break;
                case 0x18: return ld_st_vx(); break;
                case 0x1E: return add_i_vx(); break;
                case 0x29: return ld_f_vx(); break;
                case 0x33: return ld_b_vx(); break;
                case 0x55: return ld_i_vx(); break;
                case 0x65: return ld_vx_i(); break;
                default: 
                    err("INVALID INSTRUCTION");
                    return CPU_INVALID_INSTRUCTION;
                    break;
            }
            break;
    }

    return CPU_DECODE_FAILED;
}

cpu_status cpu::cls(){
    gfx_status status = _g->clear_framebuffer();
    if(status == GFX_OK)
        return CPU_OK;
    return CPU_GFX_ERR;
}

cpu_status cpu::ret(){
    _pc = _stack[_sp];
    _sp--;
    if(_sp > CHIP8_STACK_SIZE)
        return CPU_SP_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::jp(){
    _pc = _inst.nnn;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}
cpu_status cpu::call(){
    _sp++;
    _stack[_sp] = _pc;
    _pc = _inst.nnn;
    if(_sp > CHIP8_STACK_SIZE)
        return CPU_SP_OUT_OF_BOUNDS;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}
cpu_status cpu::se_vx_byte(){
    if(_reg[_inst.x] == _inst.kk)
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::sne_vx_byte(){
    if(_reg[_inst.x] != _inst.kk)
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::se_vx_vy(){
    if(_reg[_inst.x] == _reg[_inst.y])
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::ld_vx_byte(){
    _reg[_inst.x] = _inst.kk;
    return CPU_OK;
}

cpu_status cpu::add_vx_byte(){
    _reg[_inst.x] += _inst.kk;
    return CPU_OK;
}

cpu_status cpu::ld_vx_vy(){
    _reg[_inst.x] = _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::or_vx_vy(){
    _reg[_inst.x] = _reg[_inst.x] | _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::and_vx_vy(){
    _reg[_inst.x] = _reg[_inst.x] & _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::xor_vx_vy(){
    _reg[_inst.x] = _reg[_inst.x] ^ _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::add_vx_vy(){
    _reg[0xF] = (0xFF - _reg[_inst.x] < _reg[_inst.y]) ? 1 : 0;
    _reg[_inst.x] = _reg[_inst.x] + _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::sub_vx_vy(){
    _reg[0xF] = (_reg[_inst.x] > _reg[_inst.y]) ? 1 : 0;
    _reg[_inst.x] = _reg[_inst.x] - _reg[_inst.y];
    return CPU_OK;
}

cpu_status cpu::shr_vx(){
    _reg[0xF] = ((_reg[_inst.x] & 0x01) == 0x01);
    _reg[_inst.x] /= 2;
    return CPU_OK;
}

cpu_status cpu::subn_vx_vy(){
    _reg[0xF] = (_reg[_inst.y] > _reg[_inst.x]) ? 1 : 0;
    _reg[_inst.x] = _reg[_inst.y] - _reg[_inst.x];
    return CPU_OK;
}

cpu_status cpu::shl_vx(){
    _reg[0xF] = ((_reg[_inst.x] & 0x80) == 0x80);
    _reg[_inst.x] *= 2;
    return CPU_OK;
}

cpu_status cpu::sne_vx_vy(){
    if(_reg[_inst.x] != _reg[_inst.y])
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::ld_i_addr(){
    _I = _inst.nnn;
    return CPU_OK;
}

cpu_status cpu::jp_v0_addr(){
    _pc = _reg[0x0] + _inst.nnn;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::rnd_vx_byte(){
    _reg[_inst.x] = (rand() % 256) & _inst.kk;
    return CPU_OK;
}

cpu_status cpu::drw_vx_vy_n(){
    _draw = 1;
    if(DEBUG_ENABLED) print_cpu_state(); // Print CPU state twice for DRW or we will miss the draw flag
    uint8_t sprite[_inst.n];
    _m->read_mem(sprite, _I, _inst.n);
    gfx_status status = _g->draw(_reg[_inst.x], _reg[_inst.y], sprite, _inst.n, _reg);
    if(status == GFX_OK)
        return CPU_OK;
    return CPU_GFX_ERR;
}

cpu_status cpu::skp_vx(){
    if(_key[_reg[_inst.x]] == 1)
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::sknp_vx(){
    if(_key[_reg[_inst.x]] != 1)
        _pc += 2;
    if(_pc > CHIP8_MEM_SIZE)
        return CPU_PC_OUT_OF_BOUNDS;
    return CPU_OK;
}

cpu_status cpu::ld_vx_dt(){
    _reg[_inst.x] = _DT;
    return CPU_OK;
}

cpu_status cpu::ld_vx_k(){
    bool pressed = false;
    for(auto x=0; x<16; ++x){
        if(_key[x] == 1){
            pressed = true;
            _reg[_inst.x] = x;
            break;
        }
    }
    if(!pressed) _pc -= 2;
    return CPU_OK;
}

cpu_status cpu::ld_dt_vx(){
    _DT = _reg[_inst.x];
    return CPU_OK;
}

cpu_status cpu::ld_st_vx(){
    _ST = _reg[_inst.x];
    return CPU_OK;
}

cpu_status cpu::add_i_vx(){
    _I += _reg[_inst.x];
    if(_I > CHIP8_MEM_SIZE)
        warn("I LARGER THAN MEM SIZE");
    return CPU_OK;
}

cpu_status cpu::ld_f_vx(){
    _I = 5 * _reg[_inst.x];
    return CPU_OK;
}

cpu_status cpu::ld_b_vx(){
    uint8_t buf[3];
    buf[0] = _reg[_inst.x] / 100;
    buf[1] = (_reg[_inst.x] % 100) / 10;
    buf[2] = _reg[_inst.x] % 10;
    mem_status status = _m->write_mem(buf, _I, 3);
    if(status != MEM_OK){
        err("MEM FAILED");
        return CPU_MEM_ERR;
    }
    return CPU_OK;    
}

cpu_status cpu::ld_i_vx(){
    mem_status status = _m->write_mem(_reg, _I, _inst.x + 1);
    if(status != MEM_OK){
        err("MEM FAILED");
        return CPU_MEM_ERR;
    }
    return CPU_OK; 
}

cpu_status cpu::ld_vx_i(){
    mem_status status = _m->read_mem(_reg, _I, _inst.x + 1);
    if(status != MEM_OK){
        err("MEM FAILED");
        return CPU_MEM_ERR;
    }
    return CPU_OK;
}