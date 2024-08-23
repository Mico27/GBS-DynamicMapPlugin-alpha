#pragma bank 255

#include <string.h>

#include "meta_tiles.h"
#include "system.h"
#include "vm.h"
#include "bankdata.h"
#include "scroll.h"
#include "gbs_types.h"
#include "actor.h"

uint8_t __at(0xA000) sram_map_data[MAX_MAP_DATA_SIZE];

UBYTE metatile_bank;
unsigned char* metatile_ptr;

UBYTE metatile_attr_bank;
unsigned char* metatile_attr_ptr;

UBYTE metatile_collision_bank;
unsigned char* metatile_collision_ptr;

UBYTE metatile_changed_bank;
far_ptr_t * metatile_changed_ptr;

UBYTE metatile_collided_bank;
far_ptr_t * metatile_collided_ptr;

UBYTE actors_tile_top_cache[10];
UBYTE actors_tile_left_cache[10];
UBYTE actors_tile_bottom_cache[10];
UBYTE actors_tile_right_cache[10];
UBYTE current_left_tile; 
UBYTE current_top_tile;	
UBYTE current_right_tile; 
UBYTE current_bottom_tile;
	
UBYTE actors_collisionx_cache[10 * 4];
UBYTE actors_collisiony_cache[10 * 4];

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

void load_metatile_changed_events(SCRIPT_CTX * THIS) OLDCALL BANKED {
	metatile_changed_bank = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	metatile_changed_ptr = *(far_ptr_t **) VM_REF_TO_PTR(FN_ARG1);	
}

void load_metatile_collided_events(SCRIPT_CTX * THIS) OLDCALL BANKED {
	metatile_collided_bank = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	metatile_collided_ptr = *(far_ptr_t **) VM_REF_TO_PTR(FN_ARG1);	
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

inline void execute_metatile_changed_event(far_ptr_t meta_tile_event, UBYTE tile_id, UBYTE tile_x, UBYTE tile_y) {
	SCRIPT_CTX * ctx = create_script_context();
	vm_push(ctx, tile_y);
	vm_push(ctx, tile_x);
	vm_push(ctx, tile_id);
	vm_call_far(ctx, meta_tile_event.bank, meta_tile_event.ptr);
}

void on_actor_metatile_changed(UBYTE actor_idx) BANKED {
	if (!metatile_changed_bank){
		return;
	}
	far_ptr_t meta_tile_event;
    MemcpyBanked(&meta_tile_event, metatile_changed_ptr + actor_idx, sizeof(far_ptr_t), metatile_changed_bank);
	if (!meta_tile_event.bank){
		return;
	}
	actor_t *actor = (actors + actor_idx);
	
    current_left_tile = ((actor->pos.x >> 4) + actor->bounds.left) >> 3; 
    current_top_tile = ((actor->pos.y >> 4) + actor->bounds.top) >> 3;	
	current_right_tile = ((actor->pos.x >> 4) + actor->bounds.right) >> 3; 
    current_bottom_tile = ((actor->pos.y >> 4) + actor->bounds.bottom) >> 3;	
		
	UBYTE tmp_tile = 0;
	UBYTE left_side_checked = 0;
	UBYTE right_side_checked = 0;
	//check left
	if (current_left_tile < actors_tile_left_cache[actor_idx]){
		tmp_tile = current_top_tile;
		while (tmp_tile <= current_bottom_tile){
			execute_metatile_changed_event(meta_tile_event, sram_map_data[VRAM_OFFSET(current_left_tile, tmp_tile)], current_left_tile, tmp_tile);			
			tmp_tile++;
		}
		left_side_checked = 1;
	}
	//check right	
	if (current_left_tile != current_right_tile && (current_right_tile > actors_tile_right_cache[actor_idx])){
		tmp_tile = current_top_tile;
		while (tmp_tile <= current_bottom_tile){
			execute_metatile_changed_event(meta_tile_event, sram_map_data[VRAM_OFFSET(current_right_tile, tmp_tile)], current_right_tile, tmp_tile);
			tmp_tile++;
		}
		right_side_checked = 1;
	}
	
	//Check top
	if (current_top_tile < actors_tile_top_cache[actor_idx]){
		tmp_tile = current_left_tile + left_side_checked;
		while (tmp_tile <= (current_right_tile - right_side_checked)){
			execute_metatile_changed_event(meta_tile_event, sram_map_data[VRAM_OFFSET(tmp_tile, current_top_tile)], tmp_tile, current_top_tile);			
			tmp_tile++;
		}
	}	
	
	//Check bottom
	if (current_top_tile != current_bottom_tile && (current_bottom_tile > actors_tile_bottom_cache[actor_idx])){
		tmp_tile = current_left_tile + left_side_checked;
		while (tmp_tile <= (current_right_tile - right_side_checked)){
			execute_metatile_changed_event(meta_tile_event, sram_map_data[VRAM_OFFSET(tmp_tile, current_bottom_tile)], tmp_tile, current_bottom_tile);			
			tmp_tile++;
		}
	}
	
	actors_tile_left_cache[actor_idx] = current_left_tile;
	actors_tile_top_cache[actor_idx] = current_top_tile;
	actors_tile_right_cache[actor_idx] = current_right_tile;
	actors_tile_bottom_cache[actor_idx] = current_bottom_tile;
}

void on_actor_metatile_collision(UBYTE actor_idx, UBYTE tile_x, UBYTE tile_y, UBYTE direction) BANKED{
	if (!metatile_collided_bank){
		return;
	}
	far_ptr_t meta_tile_event;
    MemcpyBanked(&meta_tile_event, metatile_collided_ptr + actor_idx, sizeof(far_ptr_t), metatile_collided_bank);
	if (!meta_tile_event.bank){
		return;
	}
	UBYTE collision_cache = (actor_idx * 4) + direction;
	if (actors_collisionx_cache[collision_cache] != tile_x || actors_collisiony_cache[collision_cache] != tile_y) {
		actors_collisionx_cache[collision_cache] = tile_x;
		actors_collisiony_cache[collision_cache] = tile_y;
		
		SCRIPT_CTX * ctx = create_script_context();
		vm_push(ctx, direction);
		vm_push(ctx, tile_y);
		vm_push(ctx, tile_x);
		vm_push(ctx, sram_map_data[VRAM_OFFSET(tile_x, tile_y)]);
		vm_call_far(ctx, meta_tile_event.bank, meta_tile_event.ptr);
		
	}
}

void reset_collision_cache(UBYTE actor_idx, UBYTE direction) BANKED {
	UBYTE collision_cache = (actor_idx * 4) + direction;
	actors_collisionx_cache[collision_cache] = 0;
	actors_collisiony_cache[collision_cache] = 0;
}

SCRIPT_CTX * create_script_context(void) BANKED {
    if (free_ctxs == NULL) return NULL;
#ifdef SAFE_SCRIPT_EXECUTE
    if (pc == NULL) return NULL;
#endif

    SCRIPT_CTX * tmp = free_ctxs;
    // remove context from free list
    free_ctxs = free_ctxs->next;
    // initialize context
    tmp->stack_ptr = tmp->base_addr;
    // clear termination flag
    tmp->terminated = FALSE;
    // clear lock count
    tmp->lock_count = 0;
    // clear flags
    tmp->flags = 0;
    // Clear update fn
    tmp->update_fn_bank = 0;
    // append context to active list
    tmp->next = NULL;
    if (first_ctx) {
         SCRIPT_CTX * idx = first_ctx;
         while (idx->next) idx = idx->next;
         idx->next = tmp;
    } else first_ctx = tmp;
    // return thread ID
    return tmp;
}