#ifndef BUTTONS_H
    #define BUTTONS_H

    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
    #include <stdio.h>
    #include <stdlib.h>

    #define MAX_BUTTONS 5
    #define BUTTON_BORDER_PX 2
    #define BUTTON_PADDING 50
    #define BUTTON_HEIGTH 50
    #define WINDOW_WIDTH 1360
    #define WINDOW_HEIGTH 768

    typedef struct {
        SDL_Rect rect;
        const char* label;
        const char* command;
        Uint8 red, green, blue, alpha;
        Uint8 text_red, text_green, text_blue, text_alpha;
        Uint8 hover_red, hover_green, hover_blue, hover_alpha;
    } Button;

    typedef struct {
        Button* button_ptr;
        void* next;
    } Node;

    typedef enum {FALSE, TRUE} Bool;


    Bool is_button_hovered(Button* button_ptr, int x_coord, int y_coord);
    Node* load_config();
    void print_config(Node* head);
    void destroy_config(Node* head);
    Node* new_node();
    Button* new_button();
    void draw_buttons_and_labels(Node* config, TTF_Font* font, SDL_Renderer* renderer, int mouse_x, int mouse_y);
#endif