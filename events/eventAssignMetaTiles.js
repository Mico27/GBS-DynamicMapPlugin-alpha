export const id = "EVENT_ASSIGN_META_TILES";
export const name = "Assign meta tiles";
export const groups = ["EVENT_GROUP_MISC"];

export const autoLabel = (fetchArg) => {
  return `Assign meta tiles`;
};

export const fields = [
  {
    key: `meta_tile_symbol`,
    label: "Meta tile name",
    type: "text",
  }, 
];

export const compile = (input, helpers) => {
  
  const { _callNative, _stackPushConst, _stackPop, _addComment } = helpers;
  
  const symbol = (input.meta_tile_symbol ?? "default") + "_meta_tiles";
    
  _addComment("Assign current meta tiles data");
  
  _stackPushConst(`_${symbol}`);
  _stackPushConst(`___bank_${symbol}`);
    		
  _callNative("load_meta_tiles");
  _stackPop(2);  
};
