#include <iostream>
#include <SDL.h>
#include "cpu.h"
#include <chrono>

const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 256;

// Keypad keymap
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char ** argv){
    printf("Potato-8 v%i.%i \n", VERSION_MAJOR, VERSION_MINOR);

    if(argc < 2){
        std::cout << "Use ./potato-8 ROM.rom to run!" << std::endl;
        return 0;
    }

    cpu *c = new cpu();
    c->load_binary(argv[1]);

    SDL_Window *window = NULL;
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }else{
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL ){
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return 0;
        }
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_Texture *sdlTexture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                CHIP8_HORIZONTAL, CHIP8_VERTICAL);

    uint8_t framebuffer[CHIP8_HORIZONTAL * CHIP8_VERTICAL];
    uint32_t texture[CHIP8_HORIZONTAL * CHIP8_VERTICAL];
    SDL_Event e;

    uint64_t start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    for(;;){
        uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if(now - start > 998){
            start = now;
            cpu_status s = c->execute_instruction();
            if(s != CPU_OK)
                printf("ERR %i\n", s);
        }

        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            //SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        }

        switch(e.type){
            case SDL_KEYDOWN:
                for(int i=0; i<16; ++i){
                    if(e.key.keysym.sym == keymap[i])
                        c->set_key(i);
                }
                break;
            case SDL_KEYUP:
                for(int i=0; i<16; ++i){
                    if(e.key.keysym.sym == keymap[i])
                        c->unset_key(i);
                }
                break;

        }

        if(c->get_draw()){
            c->get_framebuffer(framebuffer);
            for(int x=0; x<(CHIP8_HORIZONTAL * CHIP8_VERTICAL); ++x){
                texture[x] = 0xFF000000 | (framebuffer[x]) * 0x00FFFFFF;
            }

            SDL_UpdateTexture(sdlTexture, NULL, texture, CHIP8_HORIZONTAL * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}