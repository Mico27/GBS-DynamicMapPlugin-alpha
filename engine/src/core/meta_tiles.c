#pragma bank 255

#include <string.h>

#include "meta_tiles.h"
#include "system.h"
#include "vm.h"
#include "bankdata.h"
#include "scroll.h"
#include "gbs_types.h"

uint8_t __at(0xA000) sram_map_data[MAX_MAP_DATA_SIZE];

UBYTE metatile_bank;
unsigned char* metatile_ptr;

UBYTE metatile_attr_bank;
unsigned char* metatile_attr_ptr;

UBYTE metatile_collision_bank;
unsigned char* metatile_collision_ptr;

void vm_load_meta_tiles(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t scene_bank = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	const scene_t * scene_ptr = *(scene_t **) VM_REF_TO_PTR(FN_ARG1);	
	scene_t scn;
    MemcpyBanked(&scn, scene_ptr, sizeof(scn), scene_bank);
	metatile_collision_bank  = scn.collisions.bank;
    metatile_collision_ptr   = scn.collisions.ptr;
	background_t bkg;
    MemcpyBanked(&bkg, scn.background.ptr, sizeof(bkg), scn.background.bank);
    metatile_bank = bkg.tilemap.bank;
    metatile_ptr = bkg.tilemap.ptr;
    metatile_attr_bank = bkg.cgb_tilemap_attr.bank;
    metatile_attr_ptr = bkg.cgb_tilemap_attr.ptr;
	
	scroll_reset();
	scroll_update();
}

void vm_replace_meta_tile(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0) & 31;
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1) & 31;
	uint8_t tile_id = *(uint8_t *) VM_REF_TO_PTR(FN_ARG2);		
	replace_meta_tile(x, y, tile_id);	
}

void vm_get_sram_tile_id_at_pos(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);
	script_memory[*(int16_t*)VM_REF_TO_PTR(FN_ARG2)] = sram_map_data[VRAM_OFFSET(x, y)];	
}

void replace_meta_tile(UBYTE x, UBYTE y, UBYTE tile_id) BANKED {
	sram_map_data[VRAM_OFFSET(x, y)] = tile_id;		
#ifdef CGB
		if (_is_CGB) {
			VBK_REG = 1;
			set_bkg_tile_xy(x, y, ReadBankedUBYTE(metatile_attr_ptr + tile_id, metatile_attr_bank));
			VBK_REG = 0;
		}
#endif
	set_bkg_tile_xy(x, y, ReadBankedUBYTE(metatile_ptr + tile_id, metatile_bank));	
}