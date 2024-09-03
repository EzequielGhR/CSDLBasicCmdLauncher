#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"

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


void* __new_t(size_t size, char* type){
    void* new = malloc(size);
    if (new == NULL){
        fprintf(stderr, "failed to allocate %s\n", type);
        exit(1);
    }

    return new;
}


Node* new_node(){
    return __new_t(sizeof(Node), "node");
}


Button* new_button(){
    return __new_t(sizeof(Button), "button");
}


Bool is_button_hovered(Button* button_ptr, int x_coord, int y_coord){
    return (x_coord > button_ptr->rect.x && x_coord < button_ptr->rect.x + button_ptr->rect.w &&
            y_coord > button_ptr->rect.y && y_coord < button_ptr->rect.y + button_ptr->rect.h);
}


void draw_buttons_and_labels(Node* config, TTF_Font* font, SDL_Renderer* renderer, int mouse_x, int mouse_y){
    Node* current = config;
    for (current; current != NULL; current = current->next){
        Button* btn_ptr = current->button_ptr;

        // Check if the mouse is hovering the button
        Bool is_hovered = is_button_hovered(btn_ptr, mouse_x, mouse_y);

        // Draw button border
        SDL_Rect border_rect = {btn_ptr->rect.x - BUTTON_BORDER_PX, btn_ptr->rect.y - BUTTON_BORDER_PX,
                                btn_ptr->rect.w + 2*BUTTON_BORDER_PX, btn_ptr->rect.h + 2*BUTTON_BORDER_PX};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &border_rect);

        // Draw button
        if (is_hovered)
            SDL_SetRenderDrawColor(renderer, btn_ptr->hover_red, btn_ptr->hover_green, btn_ptr->hover_blue, btn_ptr->hover_alpha);
        else
            SDL_SetRenderDrawColor(renderer, btn_ptr->red, btn_ptr->green, btn_ptr->blue, btn_ptr->alpha);

        SDL_RenderFillRect(renderer, &btn_ptr->rect);

        // Render label
        SDL_Color text_color = {btn_ptr->text_red, btn_ptr->text_green, btn_ptr->text_blue, btn_ptr->text_alpha};
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, btn_ptr->label, text_color);
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

        // Get text width and height
        int text_width = 0, text_height = 0;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_width, &text_height);

        // Calculate position to center text on button
        int text_x_coord = btn_ptr->rect.x + (btn_ptr->rect.w - text_width)/2;
        int text_y_coord = btn_ptr->rect.y + (btn_ptr->rect.h - text_height)/2;

        SDL_Rect text_rect = {text_x_coord, text_y_coord, text_width, text_height};
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);
    }

    SDL_RenderPresent(renderer);
}


static Button temp_button;
static int button_index = -1;


static int _config_handler(void* user, const char* section, const char* name, const char* value){
    Node** config_ptr = (Node**)user;

    // Used to compare with current sections
    char title_buffer[10];
    for (int index=0; index<MAX_BUTTONS; index++){
        // Skip until the valid section is found
        sprintf(title_buffer, "button_%d", index+1);
        if (strcmp(section, title_buffer) != 0) continue;

        if (index != button_index){
            // Redefine index so we don't redefine rect each iteration.
            button_index = index;

            // Set rect attributes
            temp_button.rect.x = BUTTON_PADDING;
            temp_button.rect.y = (2 * index + 1) * BUTTON_HEIGTH;
            temp_button.rect.w = WINDOW_WIDTH - 2 * BUTTON_PADDING;
            temp_button.rect.h = BUTTON_HEIGTH;
        }

        // Set remaining button attributes.
        if (strcmp(name, "label") == 0) temp_button.label = strdup(value);
        else if (strcmp(name, "command") == 0) temp_button.command = strdup(value);
        else if (strcmp(name, "red") == 0) temp_button.red = (Uint8)atoi(value);
        else if (strcmp(name, "green") == 0) temp_button.green = (Uint8)atoi(value);
        else if (strcmp(name, "blue") == 0) temp_button.blue = (Uint8)atoi(value);
        else if (strcmp(name, "alpha") == 0) temp_button.alpha = (Uint8)atoi(value);
        else if (strcmp(name, "hover_red") == 0) temp_button.hover_red = (Uint8)atoi(value);
        else if (strcmp(name, "hover_green") == 0) temp_button.hover_green = (Uint8)atoi(value);
        else if (strcmp(name, "hover_blue") == 0) temp_button.hover_blue = (Uint8)atoi(value);
        else if (strcmp(name, "hover_alpha") == 0) temp_button.hover_blue = (Uint8)atoi(value);
        else if (strcmp(name, "text_red") == 0) temp_button.text_red = (Uint8)atoi(value);
        else if (strcmp(name, "text_green") == 0) temp_button.text_green = (Uint8)atoi(value);
        else if (strcmp(name, "text_blue") == 0) temp_button.text_blue = (Uint8)atoi(value);

        // Set last attribute and append to list of buttons.
        else if (strcmp(name, "text_alpha") == 0){
            temp_button.text_alpha = (Uint8)atoi(value);

            // Create a new node
            Node* new = new_node();
            new->button_ptr = new_button();

            // Define the node button with the populated temp button.
            *new->button_ptr = temp_button;

            // Append new node to list
            new->next = *config_ptr;
            *config_ptr = new;
        }
    }

    return 1;
}


Node* load_config(const char* filename){
    Node* config = NULL;
    if (ini_parse(filename, _config_handler, &config) < 0){
        fprintf(stderr, "Can't load \"%s\"", filename);
        exit(1);
    }

    return config;
}


void print_config(Node* config){
    Node* current = config;
    while (current){
        printf("Label: %s; Command: %s.\n", current->button_ptr->label, current->button_ptr->command);
        current = current->next;
    }
}


void destroy_config(Node* config){
    while(config){
        Node* next = config->next;
        free(config->button_ptr);
        free(config);
        config = next;
    }
}