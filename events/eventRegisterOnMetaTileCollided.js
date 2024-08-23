export const id = "EVENT_REGISTER_ON_META_TILES_COLLIDED";
export const name = "Register on actor metatile collided";
export const groups = ["Meta Tiles"];

export const autoLabel = (fetchArg) => {
  return `Register on actor metatile collided`;
};

export const fields = [
  {
    key: "actorId",
    label: "Actor",
    description: "Actor",
    type: "actor",
    defaultValue: "$self$",
  },
  {
        label: "The first parameter of the script will be the tile ID"
  },
  {
        label: "The second parameter of the script will be the tile x position"
  },
  {
        label: "The third parameter of the script will be the tile y position"
  },
  {
        label: "The fourth parameter of the script will be the direction of the collision"
  },
  {
        label: "Note: it is not needed to assign variables to the script parameters, the parameters will be filled with the correct values on callback"
  },
  {
	key: "customEventId",
    type: "customEvent",
    label: "Script",
    description: "Script",
  },
];

const meta_tiles_events_cache = {};

export const compile = (input, helpers) => {
	
    const { writeAsset, options, compileCustomEventScript, setActorId, getActorIndex, getVariableAlias, _callNative, _stackPushConst, _stackPush, _stackPop, _addComment, _declareLocal, variableSetToScriptValue, warnings } = helpers;
  
  const { customEvents, scene, additionalScripts } = options;
  const customEvent = customEvents.find((ce) => ce.id === input.customEventId);
  if (!customEvent) {
    return;
  } 
  
  const compiledCustomEvent = compileCustomEventScript(customEvent.id);
  if (!compiledCustomEvent) {
    return;
  }

  const { scriptRef, argsLen } = compiledCustomEvent;
  
  	 //Fix script to have VM_STOP at the end instead of VM_RET_FAR_N
  const scriptData = additionalScripts[scriptRef];
  scriptData.compiledScript = scriptData.compiledScript.substring(0, scriptData.compiledScript.lastIndexOf("VM_RET_FAR_N"));
  scriptData.compiledScript = scriptData.compiledScript + "\n		VM_STOP";
     
  const meta_tile_event = {
  	script_symbol: scriptRef,
  	actor_id: getActorIndex(input.actorId),
  };
  
  const symbol = scene.symbol + "_on_metatile_collided";
  //load to scene
	_stackPushConst(`_${symbol}`);
	_stackPushConst(`___bank_${symbol}`);  		
	_callNative("load_metatile_collided_events");
	_stackPop(2);
	
  let meta_tile_events = meta_tiles_events_cache[symbol];
  if (!meta_tile_events){
	  meta_tiles_events_cache[symbol] = meta_tile_events = {};	    
  }
  meta_tile_events[meta_tile_event.actor_id] = meta_tile_event;
  const output_events = [];
  for (let i = 0; i <= scene.actors.length; i++){
	  output_events.push(meta_tile_events[i] ?? {});
  }

  const output = `#pragma bank 255
  
			#include "gbs_types.h"
			#include "data/game_globals.h"
			${output_events.map((output_event) => output_event.script_symbol ? `#include "data/${output_event.script_symbol}.h"`: '').join("\n")}

			BANKREF(${symbol})

			const struct far_ptr_t ${symbol}[] = {
				${output_events.map((output_event) => output_event.script_symbol ? `TO_FAR_PTR_T(${output_event.script_symbol})`: '{ NULL, NULL }').join(",\n")}
			};`

    writeAsset(`${symbol}.c`, output); 
	
};
