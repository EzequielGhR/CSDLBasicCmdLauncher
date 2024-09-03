#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "buttons.h"

#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
#define BUTTONS_CONFIG_PATH "/home/zeke/Documents/Repos/CSDLBasicCmdLauncher/buttons.ini"


void launch_program(const char* command, const char* button_label){
    printf("Attempting to launch %s\n", button_label);
    int status = system(command);
    if (status == -1) fprintf(stderr, "Error launching program: %s\n", command);
}

int main(int argc, char** argv){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Emulation Center", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGTH, SDL_WINDOW_SHOWN);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    Node* config = load_config(BUTTONS_CONFIG_PATH);
    print_config(config);

    Bool running = TRUE;
    SDL_Event event;

    TTF_Font *font = TTF_OpenFont(FONT_PATH, 24);
    if (!font){
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    while(running) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) running = FALSE;
            else if (event.type == SDL_MOUSEBUTTONDOWN){
                int x_coord = event.button.x;
                int y_coord = event.button.y;
                int previous_heigth = 0;

                Node* current = config;
                for (current; current != NULL; current = current->next){
                    Button* btn_ptr = current->button_ptr;
                    if (is_button_hovered(btn_ptr, x_coord, y_coord))
                        launch_program(btn_ptr->command, btn_ptr->label);
                }
            }
        }

        // Clear Screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        draw_buttons_and_labels(config, font, renderer, mouse_x, mouse_y);
    }

    printf("Closing program\n");
    destroy_config(config);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}