const wrap8Bit = (val) => (256 + (val % 256)) % 256;
const decHex = (dec) => `0x${wrap8Bit(dec).toString(16).padStart(2, "0").toUpperCase()}`;
export const id = "EVENT_CREATE_META_TILES";
export const name = "Create meta tiles";
export const groups = ["EVENT_GROUP_MISC"];

export const autoLabel = (fetchArg) => {
  return `Create meta tiles`;
};

export const fields = [
  {
    key: `meta_tile_symbol`,
    label: "Meta tile name",
    type: "text",
  }, 
];

export const compile = (input, helpers) => {
	
  const { writeAsset, options, _getAvailableSymbol } = helpers;  
  const { scene } = options;
  
  const collisions = scene.collisions || [];
  const background = scene.background;
  const tilemap = (background && background.tilemap) ? background.tilemap.data: [];
  const tilemapAttr = (background && background.tilemapAttr) ? background.tilemapAttr.data: [];
  
  const metaTiles = [];
  for (let i = 0; i < 256; i++){	  
	  const tilemap_tile_id = tilemap[i] || 0;
	  const tilemapAttr_id = tilemapAttr[i] || 0;
	  const collision_id = collisions[i] || 0;
	  metaTiles.push({ tile_id: tilemap_tile_id, tile_attributes: tilemapAttr_id, collision_data: collision_id, behavior_id: i });
  } 
  
  const symbol = (input.meta_tile_symbol ?? "default") + "_meta_tiles";

  const output = `#pragma bank 255
  
			#include "meta_tiles.h"

			BANKREF(${symbol})

			const struct meta_tile_data_t ${symbol}[] = {
				${metaTiles.map((meta_tile) => `{ .tile_id = ${decHex(meta_tile.tile_id)}, .tile_attributes =  ${decHex(meta_tile.tile_attributes)}, .collision_data =  ${decHex(meta_tile.collision_data)}, .behavior_id =  ${decHex(meta_tile.behavior_id)} }`).join(",\n")}
			};`

    writeAsset(`${symbol}.c`, output);
};
