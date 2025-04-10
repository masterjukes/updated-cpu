#ifndef VGAVARS_H
#define VGAVARS_H

#define MEMORY_SIZE  0x1000000  // 16MB
#define VRAM_START   0xFA0000   // VGA framebuffer
#define TEXTRAM_START 0xFDE800  // Text mode RAM start
#define IO_PORTS_START 0xFE0740
#define BIOS_ROM_START 0xFE0840
#define BIOS_ROM_END   0xFFFFFF
#define BIOS_ROM_SIZE 0xFFFFFF - 0xFE0840

struct registers{
    uint8_t reg8[8];
    uint32_t reg32[8];
};

static uint8_t MEMORY[MEMORY_SIZE];  // Global memory
  // Default: VGA mode

#endif // VGAVARS_H
