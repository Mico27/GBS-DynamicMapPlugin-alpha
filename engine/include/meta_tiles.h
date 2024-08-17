#ifndef META_TILES_H
#define META_TILES_H

#define VRAM_OFFSET(x,y)  (((y & 31) << 5) + (x & 31))

#include <gbdk/platform.h>
#include "gbs_types.h"

#define MAX_MAP_DATA_SIZE 1024 //32x32 basically mirrors the tilemap VRAM data

typedef struct meta_tile_data_t {
    uint8_t tile_id;
	uint8_t tile_attributes;
	uint8_t collision_data;
	uint8_t behavior_id;
} meta_tile_data_t;

extern uint8_t __at(0xA000) sram_map_data[MAX_MAP_DATA_SIZE];
extern uint8_t curr_meta_tile_bank;
extern const meta_tile_data_t* curr_meta_tile_ptr;

uint8_t get_metatile_tile_id(uint16_t vram_tile_idx) BANKED;
uint8_t get_metatile_attr(uint16_t vram_tile_idx) BANKED;
uint8_t get_metatile_collision(uint16_t vram_tile_idx) BANKED;

#endif
