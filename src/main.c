#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BUTTONS 5
#define BUTTON_BORDER_PX 2
#define BUTTON_PADDING 50
#define BUTTON_HEIGTH 50
#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
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

typedef enum {false, true} bool;


bool is_button_hovered(Button* button_ptr, int x_coord, int y_coord);
void launch_program(const char* command, const char* button_label);
void draw_buttons_and_labels(Node* config, TTF_Font* font, SDL_Renderer* renderer, int mouse_x, int mouse_y);
void arrange_buttons(Button* buttons_array, int num_buttons);
Node* load_config();
void print_config(Node* head);
void destroy_config(Node* head);
Node* new_node();
Button* new_button();


int main(int argc, char** argv){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Emulation Center", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGTH, SDL_WINDOW_SHOWN);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    Node* config = load_config();
    print_config(config);

    bool running = true;
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
            if (event.type == SDL_QUIT) running = false;
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


bool is_button_hovered(Button* button_ptr, int x_coord, int y_coord){
    return (x_coord > button_ptr->rect.x && x_coord < button_ptr->rect.x + button_ptr->rect.w &&
            y_coord > button_ptr->rect.y && y_coord < button_ptr->rect.y + button_ptr->rect.h);
}


void launch_program(const char* command, const char* button_label){
    printf("Attempting to launch %s\n", button_label);
    int status = system(command);
    if (status == -1) fprintf(stderr, "Error launching program: %s\n", command);
}


void draw_buttons_and_labels(Node* config, TTF_Font* font, SDL_Renderer* renderer, int mouse_x, int mouse_y){
    Node* current = config;
    for (current; current != NULL; current = current->next){
        Button* btn_ptr = current->button_ptr;

        // Check if the mouse is hovering the button
        bool is_hovered = is_button_hovered(btn_ptr, mouse_x, mouse_y);

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


void arrange_buttons(Button* buttons_array, int num_buttons){
    printf("Arranging buttons\n");
    for (int i=0; i<num_buttons; i++) buttons_array[i].rect.y = (2*i+1) * BUTTON_HEIGTH;
}


Node* load_config() {
    // TODO: This needs to be set dynamically. maybe a .ini file?
    Button buttons_array[] = {
        {
            rect: {BUTTON_PADDING, BUTTON_HEIGTH, WINDOW_WIDTH - 2*BUTTON_PADDING, BUTTON_HEIGTH},
            label: "Open Terminal",
            command: "gnome-terminal",
            red: 0, green: 0, blue: 255, alpha: SDL_ALPHA_OPAQUE,
            text_red: 255, text_green: 255, text_blue: 255, text_alpha: SDL_ALPHA_OPAQUE,
            hover_red: 255, hover_green: 0, hover_blue: 0, hover_alpha: 32
        },
        {
            rect: {BUTTON_PADDING, BUTTON_HEIGTH, WINDOW_WIDTH - 2*BUTTON_PADDING, BUTTON_HEIGTH},
            label: "Open Browser",
            command: "brave-browser",
            red: 0, green: 0, blue: 255, alpha: SDL_ALPHA_OPAQUE,
            text_red: 255, text_green: 255, text_blue: 255, text_alpha: SDL_ALPHA_OPAQUE,
            hover_red: 255, hover_green: 0, hover_blue: 0, hover_alpha: 32
        },
    };

    int num_buttons = sizeof(buttons_array)/sizeof(buttons_array[0]);
    int max_buttons = (num_buttons < MAX_BUTTONS) ? num_buttons : MAX_BUTTONS;

    Node* head = NULL;

    for (int i=0; i<max_buttons; i++){
        Node* new = new_node();

        new->button_ptr = new_button();

        *new->button_ptr = buttons_array[i];
        new->button_ptr->rect.y = (2*i+1)*BUTTON_HEIGTH;

        new->next = head;
        head = new;
    }

    return head;
}


void print_config(Node* head){
    Node* current = head;
    while (current){
        printf("Label: %s; Command: %s.\n", current->button_ptr->label, current->button_ptr->command);
        current = current->next;
    }
}


void destroy_config(Node* head){
    while(head){
        Node* next = head->next;
        free(head->button_ptr);
        free(head);
        head = next;
    }
}


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