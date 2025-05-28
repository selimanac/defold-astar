---@class astar : table
---@field SOLVED number Represents a solved path result.
---@field NO_SOLUTION number Represents a result where no path is found.
---@field START_END_SAME number Represents a result where the start and end points are the same.
---@field DIRECTION_FOUR number Represents the four-direction movement option.
---@field DIRECTION_EIGHT number Represents the eight-direction movement option.
astar = {}

---@return number map_id ID of a new map.
function astar.new_map_id() end

---@param map_id number ID of the map to delete. Has to be an integer from 0 to 2^16 - 1.
function astar.delete_map(map_id) end

---@param map_width number Width of your map.
---@param map_height number Height of your map.
---@param direction number Movement direction (astar.DIRECTION_FOUR or astar.DIRECTION_EIGHT).
---@param allocate number Number of states to allocate.
---@param typical_adjacent number Typical number of adjacent states.
---@param cache? boolean Turn on path caching. Default is true.
---@param use_zero? boolean Toggle index start at 0. Default is false.
---@param map_vflip? boolean Flip the map vertically. Default is false.
---@param map_id? number ID of the map. Default is 0.
function astar.setup(map_width, map_height, direction, allocate, typical_adjacent, cache, use_zero, map_vflip, map_id) end

---@param toggle boolean true/false.
---@param map_id? number ID of the map. Default is 0.
function astar.use_zero(toggle, map_id) end

---@param world table Your tilemap data.
---@param map_id? number ID of the map. Default is 0.
function astar.set_map(world, map_id) end

--- Flip the map vertically (does not flip coordinates).
---@param map_id? number ID of the map. Default is 0.
function astar.map_vflip(map_id) end

--- Flip the map horizontally (does not flip coordinates).
---@param map_id? number ID of the map. Default is 0.
function astar.map_hflip(map_id) end

--- Set the map coordinate type.
-- @param type (number) One of astar.GRID_CLASSIC, astar.HEX_ODDR, astar.HEX_EVENR, astar.HEX_ODDQ, or astar.HEX_EVENQ.
---@param map_id? number ID of the map. Default is 0.
function astar.set_map_type(type, map_id) end

--- Define entity tile IDs that are not passable.
-- @param entities (table) List of entity tile IDs (e.g., {1, 2}).
---@param map_id? number ID of the map. Default is 0.
function astar.set_entities(entities, map_id) end

--- Enable or disable entity use during solve.
-- @param toggle (boolean) Whether to include entities in pathfinding.
---@param map_id? number ID of the map. Default is 0.
function astar.use_entities(toggle, map_id) end

---@param costs table Table of costs for directions.
---@param map_id? number ID of the map. Default is 0.
function astar.set_costs(costs, map_id) end

---@param start_x number Start tile X.
---@param start_y number Start tile Y.
---@param end_x number End tile X.
---@param end_y number End tile Y.
---@param map_id? number ID of the map. Default is 0.
---@return number result, number size, number total_cost, table path
function astar.solve(start_x, start_y, end_x, end_y, map_id) end

---@param start_x number Start tile X.
---@param start_y number Start tile Y.
---@param max_cost number Maximum cost for finding neighbours.
---@param map_id? number ID of the map. Default is 0.
---@return number near_result, number near_size, table nears
function astar.solve_near(start_x, start_y, max_cost, map_id) end

---@param map_id? number ID of the map. Default is 0.
function astar.reset_cache(map_id) end

---@param x number Tile X.
---@param y number Tile Y.
---@param map_id? number ID of the map. Default is 0.
---@return number value
function astar.get_at(x, y, map_id) end

---@param x number Tile X.
---@param y number Tile Y.
---@param value number Value to set.
---@param map_id? number ID of the map. Default is 0.
function astar.set_at(x, y, value, map_id) end

--- Print the current map to the console for debugging.
---@param map_id? number ID of the map. Default is 0.
function astar.print_map(map_id) end

--- Fully reset the pathfinding system and free memory.
function astar.reset() end
