#include <SDL2/SDL_events.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "headers/vgavars.h"
#include "headers/sdl_draw.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int PC;
bool F_equal = false;
bool F_Nequal = false;
bool F_greater = false;
bool F_less = false;
bool F_greatoreq = false;
bool F_lessoreq = false;
uint32_t val1;
uint32_t val2;
uint32_t result;
uint8_t instruction;
uint8_t operand_1;
uint8_t operand_2;
uint8_t operand_3;
uint8_t operand_4;
uint8_t operand_5;
int window_width;
int window_height;
int i;
SDL_DrawContext *ctx;
int tick;

uint8_t *hdd_memory = NULL;  // Pointer to HDD data in RAM

void initdisk() {
    FILE *f = fopen("hdd.img", "rb");
    if (!f) {
        perror("Failed to open HDD image");
        exit(1);
    }

    // Allocate memory for the HDD image
    hdd_memory = malloc(33554432);
    if (!hdd_memory) {
        perror("Failed to allocate memory for HDD image");
        fclose(f);
        exit(1);
    }

    // Read the full HDD image into RAM
    fread(hdd_memory, 1, 33554432, f);
    fclose(f);
}

struct registers regs;


uint8_t readreg8(uint8_t reg){
    if (reg > 7){
        return 0;
    }

    return regs.reg8[reg];
}

uint32_t readreg32(uint8_t reg){
    if (reg > 7){
        return 0;
    }

    return regs.reg32[reg];
}

uint8_t writereg8(uint8_t reg, uint8_t value){
    if (reg > 7){
        return 0;
    }

    regs.reg8[reg] = value;
}

uint8_t writereg32(uint8_t reg, uint32_t value){
    if (reg > 7){
        return 0;
    }

    regs.reg32[reg] = value;
}





uint8_t readmemory8(uint32_t addr){
    if (addr >= BIOS_ROM_END){
        return 0;
    }
    return MEMORY[addr];
}

uint32_t readmemory32(uint32_t addr){
    if (addr >= BIOS_ROM_END){
        return 0;
    }
    return ((MEMORY[addr] << 24) | (MEMORY[addr+1] << 16) | (MEMORY[addr+2] << 8) | MEMORY[addr+3]);
}


void writememory8(uint32_t addr,uint8_t value){
    if (addr > BIOS_ROM_END){
        return;
    }

    if(addr < BIOS_ROM_START){
        MEMORY[addr] = value;
    }
}

void writememory32(uint32_t addr, uint32_t value){         
    if (addr >= BIOS_ROM_END){
        return;
    }


    if(addr < BIOS_ROM_START){
        MEMORY[addr] = (value >> 24) & 0xff;
        MEMORY[addr+1] = (value >> 16) & 0xff;
        MEMORY[addr+2] = (value >> 8) & 0xff;
        MEMORY[addr+3] = (value >> 0) & 0xff;
    }
}
  // Buffer for the sector

void readdisk() {
    uint16_t sector = (readmemory8((IO_PORTS_START+4)) << 8) | readmemory8(IO_PORTS_START+5);
    uint32_t address = (readmemory8(IO_PORTS_START+6) << 24) | (readmemory8(IO_PORTS_START+7) << 16) | (readmemory8(IO_PORTS_START+8) << 8)  | readmemory8(IO_PORTS_START+9);

    // Write to RAM using writememory8()
    for (int i = 0; i < 512; i++) {
        writememory8(address + i, hdd_memory[(sector*512)+i]);
    }
}

void writedisk() {
    uint16_t sector = (readmemory8((IO_PORTS_START+4)) << 8) | readmemory8(IO_PORTS_START+5);
    uint32_t address = (readmemory8(IO_PORTS_START+6) << 24) | (readmemory8(IO_PORTS_START+7) << 16) | (readmemory8(IO_PORTS_START+8) << 8)  | readmemory8(IO_PORTS_START+9);

    // Write to RAM using writememory8()
    for (int i = 0; i < 512; i++) {
        hdd_memory[(sector*512)+i] = readmemory8(address+i);
    }
}




void execute(uint8_t ins,uint8_t op1,uint8_t op2,uint8_t op3,uint8_t op4,uint8_t op5){
    //printf("OP:%d OPAND1:%d OPAND2:%d OPAND3:%d OPAND4:%d OPAND5:%d \n", ins,op1,op2,op3,op4,op5);
    switch(ins){
        case 0x1: //imm8 to reg8
            writereg8(op1,op2);
            break;
        case 0x2: //imm32 to reg32
            writereg32(op1, (op2 << 24) | (op3 << 16) | (op4 << 8) | op5);
            break; 
        case 0x3: //reg8 to mem8
            writememory8((op1 << 24) | (op2 << 16) | (op3 << 8) | op4, readreg8(op5));
            break; 
        case 0x4: //reg32 to mem32
            writememory32((op1 << 24) | (op2 << 16) | (op3 << 8) | op4, readreg32(op5));
            break; 
        case 0x5: //mem8 to reg8
            writereg8(op1,readmemory8((op2 << 24) | (op3 << 16) | (op4 << 8) | op5));
            break;
        case 0x6: //mem32 to reg32
            writereg32(op1,readmemory32((op2 << 24) | (op3 << 16) | (op4 << 8) | op5));
            break; 
        case 0x7: //reg8 to reg8
            writereg8(op1,readreg8(op2));
            break; 
        case 0x8: //reg32 to reg32
            writereg32(op1,readreg32(op2));
            break; 
        case 0x9: //reg32 to mem(reg32)
            writememory8(readreg32(op1),readreg32(op2));
            break;  
        case 0xA: //mem(reg32) to reg32 
            writereg8(op1,readmemory8(readreg32(op2)));
            break; 
        case 0xB: //ADD8
            writereg8(op1,(uint8_t)(readreg8(op2)+readreg8(op3)));
            break; 
        case 0xC: //SUB8
            writereg8(op1,(uint8_t)(readreg8(op2)-readreg8(op3)));
            break; 
        case 0xD: //DIV8
            if (readreg8(op3) != 0) {writereg8(op1,(uint8_t)(readreg8(op2)/ readreg8(op3)));}
            else {writereg8(op1,0);}
            break; 
        case 0xE: //MULT8
            writereg8(op1,(uint8_t)(readreg8(op2) * readreg8(op3)));
            break; 
        case 0xF: //ADD32
            writereg32(op1,(uint32_t)(readreg32(op2)+readreg32(op3)));
            break; 
        case 0x10: //SUB32
            writereg32(op1,(uint32_t)(readreg32(op2)-readreg32(op3)));
            break; 
        case 0x11: //DIV32
            if (readreg32(op3) != 0) {writereg32(op1,(uint32_t)(readreg32(op2)/readreg32(op3)));}
            else {writereg32(op1,0);}
            break; 
        case 0x12: //MULT32
            writereg32(op1,(uint32_t)(readreg32(op2)*readreg32(op3)));
            break; 
        case 0x13: //JMP
            PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;
            break; 
        case 0x14: //JEQ
            if (F_equal){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            break; 
        case 0x15: //JNE
            if (F_Nequal){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            break; 
        case 0x16: //JLT
            if (F_less){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            break; 
        case 0x17: //JGT
            if (F_greater){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            //printf("%04x %04x %04x %04x \n", op1, op2, op3, op4);
            //printf("%04x\n", PC);
            break;
        case 0x18: //JLE
            if (F_lessoreq){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            break; 
        case 0x19: //JGE
            if (F_greatoreq){PC = (op1 << 24) | (op2 << 16) | (op3 << 8) | op4;} else {PC += 6;}
            break; 
        case 0x1A: //AND8
            writereg8(op1, readreg8(op2) & readreg8(op3));
            break; 
        case 0x1B: //OR8
            writereg8(op1, readreg8(op2) | readreg8(op3));
            break; 
        case 0x1C: //NOT8
            writereg8(op1, ~readreg8(op2));
            break; 
        case 0x1D: //XOR8
            writereg8(op1, readreg8(op2) ^ readreg8(op3));
            break; 
        case 0x1E: //CMP8
            val1 = readreg8(op1);
            val2 = readreg8(op2);
            result = val1 - val2;

            F_equal = (result == 0);
            F_Nequal = !F_equal;
            F_greater = (val1 > val2);
            F_less = (val1 < val2);
            F_greatoreq = (val1 >= val2);
            F_lessoreq = (val1 <= val2);
            break;
        case 0x1F: //AND32
            writereg32(op1, readreg32(op2) & readreg32(op3));
            break; 
        case 0x20: //OR32
            writereg32(op1, readreg32(op2) | readreg32(op3));
            break; 
        case 0x21: //NOT32
            writereg32(op1, ~readreg32(op2));
            break; 
        case 0x22: //XOR32
            writereg32(op1, readreg32(op2) ^ readreg32(op3));
            break; 
        case 0x23: //CMP32
            val1 = readreg32(op1);
            val2 = readreg32(op2);
            result = val1 - val2;

            F_equal = (result == 0);
            F_Nequal = !F_equal;
            F_greater = (val1 > val2);
            F_less = !F_greater;
            F_greatoreq = F_greater || F_equal;
            F_lessoreq = F_less || F_equal;
            break;
        case 0x24: //INC8
            writereg8(op1, readreg8(op2)+1);
            break; 
        case 0x25: //DEC8
            writereg8(op1, readreg8(op2)-1);
            break;  
        case 0x26: //SHL8
            writereg8(op1, readreg8(op2) << readreg8(op3));
            break;  
        case 0x27: //SRL8
            writereg8(op1, readreg8(op2) >> readreg8(op3));
            break;
        case 0x28: //INC32
            writereg32(op1, readreg32(op2)+1);
            break; 
        case 0x29: //DEC32
            writereg32(op1, readreg32(op2)-1);
            break;  
        case 0x2A: //SHL32
            writereg32(op1, readreg32(op2) << readreg32(op3));
            break;  
        case 0x2B: //SRL32
            writereg32(op1, readreg32(op2) >> readreg32(op3));
            break;
        case 0x2C: //HLT
            exit(0);
            break;
        case 0x2D: //imm8 to memory8
            writememory8((op1 << 24) | (op2 << 16) | (op3 << 8) | op4, op5);
            break;
        default:
            //NO OP//
            break;  
    }
}
uint8_t BIOSROM[BIOS_ROM_END-BIOS_ROM_START];
int loadBIOSROM(const char *filename) {
    FILE *file = fopen(filename, "rb");  // Open the file in binary mode
    if (!file) {
        perror("Failed to open BIOS file");
        return -1;  // Return an error if file couldn't be opened
    }

    size_t bytesRead = fread(BIOSROM, 1, BIOS_ROM_SIZE, file);  // Read data into BIOSROM array
    if (bytesRead != BIOS_ROM_SIZE) {
        if (feof(file)) {
            printf("Warning: BIOS file is smaller than expected.\n");
        } else {
            perror("Error reading BIOS file");
            fclose(file);
            return -1;  // Return an error if reading failed
        }
    }

    fclose(file);  // Close the file after reading
    return 0;  // Success
}

void setup_boot(){
    const char *biosFilePath = "/home/alfie/Projects/VGAEMU/bios.bin";
    loadBIOSROM(biosFilePath);
    for(int i = BIOS_ROM_START; i < BIOS_ROM_END; i++){
        MEMORY[i] = BIOSROM[i-BIOS_ROM_START];
    }
    PC = BIOS_ROM_START;   
}

void CPUCLOCK(){

    if ((PC >= VRAM_START) && (PC < BIOS_ROM_START-1)){
        PC = 0;
    }
    if (PC > BIOS_ROM_END) {PC = 0;}

    instruction = readmemory8(PC);  
    operand_1 = readmemory8(PC+1);
    operand_2 = readmemory8(PC+2);
    operand_3 = readmemory8(PC+3);
    operand_4 = readmemory8(PC+4);
    operand_5 = readmemory8(PC+5);
    
    execute(instruction,operand_1,operand_2,operand_3,operand_4,operand_5);
    //if (instruction != 0) {printf("ADDR:%d OP:%d %d %d %d %d %d CHAR: %d COLOR: %d \n",PC,instruction,operand_1,operand_2,operand_3,operand_4,operand_5,MEMORY[TEXTRAM_START],MEMORY[TEXTRAM_START+1]);}
    //printf("EQ %d, NEQ %d, GT %d, GTE %d, LT %d, LTE %d \n", F_equal, F_Nequal, F_greater, F_greatoreq, F_less, F_lessoreq);

    if (instruction == 0x2 || instruction == 0x3 || instruction == 0x4 || instruction == 0x5 || instruction == 0x6 || instruction == 0x2D){
        PC += 6;
    } //32 bit instructions needing 6 bytes instead of the usual 4
    else{
        if (instruction == 0x13 || instruction == 0x14 || instruction == 0x15 || instruction == 0x16 || instruction == 0x17 || instruction == 0x18 || instruction == 0x19){ //jump instructions so dont change PC
            return;
        }
        else{
            PC += 4;
        }
    }
}


SDL_DrawContext *sdl_init(const char *title, int width, int height, const char *font_path, int font_size) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return NULL;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return NULL;
    }

    SDL_DrawContext *ctx = malloc(sizeof(SDL_DrawContext));
    if (!ctx) return NULL;

    ctx->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
    if (!ctx->window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        free(ctx);
        return NULL;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx->renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        free(ctx);
        return NULL;
    }

    ctx->font = TTF_OpenFont(font_path, font_size);
    if (!ctx->font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void sdl_draw_pixel(SDL_DrawContext *ctx, int x, int y, SDL_Color color) {
    SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
    SDL_GetWindowSurface(ctx->window);
    SDL_RenderDrawPoint(ctx->renderer, x, y);
}

void sdl_draw_text(SDL_DrawContext *ctx, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(ctx->font, text, color);
    SDL_GetWindowSurface(ctx->window);
    if (!surface) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(ctx->renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void sdl_clear(SDL_DrawContext *ctx, SDL_Color color) {
    SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(ctx->renderer);
}

void sdl_present(SDL_DrawContext *ctx) {
    SDL_RenderPresent(ctx->renderer);
}

void sdl_destroy(SDL_DrawContext *ctx) {
    if (!ctx) return;

    if (ctx->font) TTF_CloseFont(ctx->font);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window) SDL_DestroyWindow(ctx->window);

    TTF_Quit();
    SDL_Quit();

    free(ctx);
}

const SDL_Color color_palette[16] = {
    {0, 0, 0, 255},     // Black
    {0, 0, 255, 255},   // Blue
    {0, 255, 0, 255},   // Green
    {0, 170, 170, 255}, // Cyan
    {255, 0, 0, 255},   // Red
    {170, 0, 170, 255}, // Magenta
    {170, 85, 0, 255},  // Brown (Dark Yellow)
    {170, 170, 170, 255}, // Light Gray
    {85, 85, 85, 255},  // Dark Gray
    {85, 85, 255, 255}, // Light Blue
    {85, 255, 85, 255}, // Light Green
    {85, 255, 255, 255}, // Light Cyan
    {255, 85, 85, 255}, // Light Red
    {255, 85, 255, 255}, // Light Magenta
    {255, 255, 85, 255}, // Yellow
    {255, 255, 255, 255}  // White
};

int startup(SDL_DrawContext **ctx) {
    *ctx = sdl_init("VGA OUTPUT", 640, 400, "/usr/share/fonts/TTF/PxPlus_IBM_EGA_8x8.ttf", 8);
    if (!*ctx) return 1;
    return 0;
}

/*
void setchar(char charcter, int bg, int fg, int col, int row){
    mode = true;
    MEMORY[((row*80+col)*2)+TEXTRAM_START] = charcter;
    MEMORY[((row*80+col)*2)+TEXTRAM_START+1] = ((bg << 4) &0xFF) | (fg &0xFF); 
    //printf("Char: %c   fg: %d   bg: %d   addr: %d\n", charcter, fg, bg, (row*80+col)+TEXTRAM_START);
}

int chartest(char str[],int bg, int fg, int col, int row){
    for (int num=0;num<strlen(str);num++){
        setchar(str[num],bg,fg,col+num,row);
    }
    return 0;
}

void runcode(){
    chartest("               VGA TEST                 ",15,1,0,0);
    chartest("              80x50 COLOR               ",2,0,0,2);
    chartest("This is an example of a simple VGA text!",15,0,0,4);
    chartest("VGA text mode is running in a 40x25 Reso",15,0,0,5);
    chartest("lution, with support for",15,0,0,6);
    chartest("colors",4,1,26,6);
    chartest("in a ",15,0,34,6);
    chartest("standard VGA 8x8 ASCII Font",15,0,0,7);
}
*/
void updatescreen(){
    if (MEMORY[IO_PORTS_START] == 0xFF) { // Text mode
        sdl_clear(ctx, (SDL_Color){0, 0, 0, 255});

        for (int row = 0; row < 8000; row+=2) {
            uint8_t char_ = MEMORY[row+TEXTRAM_START];
            uint8_t color = MEMORY[row + TEXTRAM_START + 1];
            uint8_t foreground = (color >> 4) & 0x0F; //color = 1101 1001, fg = 1101, bg = 1001
            uint8_t background = color & 0x0F;

            char chartr[2] = {(char)char_, '\0'};
                // Scale the position and size based on window size
            int char_width = window_width / 80;
            int char_height = window_height / 50;
            int x = ((row/2) %80) * char_width;
            int y = floor(((row/2)/80)) * char_height;

                /*if (char_ != 0 || foreground != 0 || background != 0){
                    printf("char: %c  fg: %d   bg:  %d   x: %d    y:  %d   addr:  %d\n", char_, foreground, background,x,y, row+TEXTRAM_START);
                    printf("row: %d  wdth: %d  hght: %d\n",row,window_width, window_height);
                }*/

            SDL_Rect bg_rect = {x, y, char_width, char_height};
            SDL_SetRenderDrawColor(ctx->renderer, 
            color_palette[background].r, 
            color_palette[background].g, 
            color_palette[background].b, 
            color_palette[background].a);
            SDL_RenderFillRect(ctx->renderer, &bg_rect);
            sdl_draw_text(ctx, chartr, x+1, y+1, color_palette[foreground]);
        }
    }
        

    if (MEMORY[IO_PORTS_START] != 0xFF) { // VGA mode
        sdl_clear(ctx, (SDL_Color){0, 0, 0, 255});
        for (i = 0; i < 256000; i++) {
            int x = (i % 640) * (window_width / 640);
            int y = (i / 640) * (window_height / 400);

            uint8_t r = (MEMORY[i+VRAM_START] >> 5) & 0x07;
            uint8_t g = (MEMORY[i+VRAM_START] >> 2) & 0x07;
            uint8_t b = (MEMORY[i+VRAM_START]) & 0x03;
            r = (r * 255) / 7;    int window_width = 640;
            int window_height = 400;
            g = (g * 255) / 7;
            b = (b * 255) / 3;

            SDL_Rect pixel_rect = {x, y, window_width / 640, window_height / 400};
            SDL_SetRenderDrawColor(ctx->renderer, r, g, b, 255);
            SDL_RenderFillRect(ctx->renderer, &pixel_rect);
        }
    }
}
void save_hdd_to_file() {
    if (!hdd_memory) {
        perror("HDD memory not loaded");
        return;
    }

    FILE *f = fopen("hdd.img", "wb");  // Open file in write mode
    if (!f) {
        perror("Failed to open HDD image for writing");
        return;
    }

    // Write the entire HDD memory to file
    if (fwrite(hdd_memory, 1, 33554432, f) != 33554432) {
        perror("Failed to write HDD image");
    }

    fclose(f);
}



void handle_sigint(int sig) {
    printf("Received SIGINT. Saving HDD to file...\n");
    save_hdd_to_file();  // Save the memory to disk when Ctrl+C is pressed
    free(hdd_memory);    // Free the allocated memory
    exit(0);  // Exit the program cleanly
}

int main() {
    int i;
    bool running = true;
    SDL_Event event; 
    if (startup(&ctx)) return 1;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 0, 0, 255};
    setup_boot();
    initdisk();
    signal(SIGINT, handle_sigint);

    while (running) {
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_KEYDOWN) {
                uint8_t keycode = event.key.keysym.scancode;
                writememory8(IO_PORTS_START+10, keycode);
                printf("%d \n",keycode);
            }
            if (event.type == SDL_KEYUP) {
                writememory8(IO_PORTS_START+10, 0);
            }
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window_width = event.window.data1;
                    window_height = event.window.data2;

                    // Dynamically reload font size proportional to window height
                    int new_font_size = window_height / 50; // One font cell per row
                    TTF_CloseFont(ctx->font);
                    ctx->font = TTF_OpenFont("/usr/share/fonts/TTF/PxPlus_IBM_EGA_8x8.ttf", new_font_size);
                    if (!ctx->font) {
                        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
                        running = 0;
                    }
                }
            }
        }
        
        CPUCLOCK();

        if (readmemory8(IO_PORTS_START+3) == 1){
            writedisk();
            writememory8(IO_PORTS_START+3,0);
        }
        if (readmemory8(IO_PORTS_START+3) == 2){
            readdisk();
            writememory8(IO_PORTS_START+3,0);
        }

        
        if (tick == 20000){
            updatescreen();
            sdl_present(ctx);
            tick = 0;
        }
        tick += 1;
    }

    sdl_destroy(ctx);
    return 0;
}







