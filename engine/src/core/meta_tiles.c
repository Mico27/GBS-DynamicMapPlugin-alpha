#pragma bank 255

#include <string.h>

#include "meta_tiles.h"
#include "system.h"
#include "vm.h"
#include "bankdata.h"
#include "scroll.h"

uint8_t __at(0xA000) sram_map_data[MAX_MAP_DATA_SIZE];

uint8_t curr_meta_tile_bank;
const meta_tile_data_t * curr_meta_tile_ptr;

void load_meta_tiles(SCRIPT_CTX * THIS) OLDCALL BANKED {
	curr_meta_tile_bank = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	curr_meta_tile_ptr = *(meta_tile_data_t **) VM_REF_TO_PTR(FN_ARG1);
	scroll_reset();
	scroll_update();
}

void replace_meta_tile(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0) & 31;
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1) & 31;
	uint8_t tile_id = *(uint8_t *) VM_REF_TO_PTR(FN_ARG2);	
	uint16_t vram_tile_idx = VRAM_OFFSET(x, y);
	sram_map_data[vram_tile_idx] = tile_id;
	
#ifdef CGB
		if (_is_CGB) {
			VBK_REG = 1;
			set_bkg_tile_xy(x, y, get_metatile_attr(vram_tile_idx));
			VBK_REG = 0;
		}
#endif
	set_bkg_tile_xy(x, y, get_metatile_tile_id(vram_tile_idx));
}

void get_sram_tile_id_at_pos(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);
	script_memory[*(int16_t*)VM_REF_TO_PTR(FN_ARG2)] = sram_map_data[VRAM_OFFSET(x, y)];	
}

uint8_t get_metatile_tile_id(uint16_t vram_tile_idx) BANKED {
	if (curr_meta_tile_bank){
		UBYTE metatile_id = sram_map_data[vram_tile_idx];
		uint8_t tile_id;
		MemcpyBanked(&tile_id, curr_meta_tile_ptr + metatile_id, 1, curr_meta_tile_bank);
		return tile_id;
	}
	return 0;
}

uint8_t get_metatile_attr(uint16_t vram_tile_idx) BANKED {
	if (curr_meta_tile_bank){
		UBYTE metatile_id = sram_map_data[vram_tile_idx];
		meta_tile_data_t meta_tile;
		MemcpyBanked(&meta_tile, curr_meta_tile_ptr + metatile_id, sizeof(meta_tile_data_t), curr_meta_tile_bank);
		return meta_tile.tile_attributes;
	}
	return 0;
}

uint8_t get_metatile_collision(uint16_t vram_tile_idx) BANKED {	
	if (curr_meta_tile_bank){
		UBYTE metatile_id = sram_map_data[vram_tile_idx];
		meta_tile_data_t meta_tile;
		MemcpyBanked(&meta_tile, curr_meta_tile_ptr + metatile_id, sizeof(meta_tile_data_t), curr_meta_tile_bank);
		return meta_tile.collision_data;
	}
	return 0;
}