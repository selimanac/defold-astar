![A*](https://github.com/selimanac/defold-astar/blob/master/assets/defoldheader.png?raw=true)

This is a path finder and A* solver (astar or a-star) native extension for [Defold Engine](https://www.defold.com/) build on [MicroPather](https://github.com/leethomason/MicroPather). 


## Installation

You can use the A* extension in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/).  
Open your game.project file and in the dependencies field under project add:

>https://github.com/selimanac/defold-astar/archive/master.zip


## Examples

https://github.com/selimanac/defold-astar-examples


## Forum

https://forum.defold.com/t/a-native-extension/


## Release Notes

1.0.1  

Small bug fixes. [More details](https://forum.defold.com/t/a-native-extension/60405/9?u=selimanac) 

1.0  
initial
 
---

# API

## astar.setup(`map_width`, `map_height`, `direction`, `allocate`, `typical_adjacent`, `cache`)

Initial setup

**PARAMETERS**
* ```map_width``` (int) - Width of your map. This is generally width of your tilemap.  
* ```map_height``` (int) - Height of your map. This is generally width of your tilemap. 
* ```direction``` (enum) - Movement direction (astar.DIRECTION_FOUR or astar.DIRECTION_EIGHT)

	**astar.DIRECTION_FOUR**: On a square grid that allows 4 directions of movement using [Manhattan distance](http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html#manhattan-distance)  
	**astar.DIRECTION_EIGHT**: On a square grid that allows 8 directions of movement using [Euclidean distance](http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html#euclidean-distance)

* ```allocate``` (int) - How many states should be internally allocated at a time. This can be hard to get correct. The higher the value, the more memory Patfinder will use.

	- If you have a small map (a few thousand states?) it may make sense to pass in the maximum value. This will cache everything, and MicroPather will only need one main memory allocation. For a chess board, allocate  would be set to 8x8 (64)
	- If your map is large, something like 1/4 the number of possible states is good.
	- If your state space is huge, use a multiple (5-10x) of the normal path. "Occasionally" call `astar.reset_cache()` to free unused memory.

* ```typical_adjacent``` (int) - Used to determine cache size. The typical number of adjacent states to a given state. (On a chessboard, 8.) Higher values use a little more memory.

* ```cache``` (bool) - Turn on path caching. Uses more memory (yet again) but at a huge speed advantage if you may call the pather with the same path or sub-path, which is common for pathing over maps in games.

**EXAMPLE**

```lua

local map_width = 5
local map_height = 5
local direction = astar.DIRECTION_EIGHT
local allocate = map_width * map_height
local typical_adjacent = 8
local cache = true

astar.setup(map_width, map_height, direction, allocate, typical_adjacent, cache)

```

## astar.set_map(`world`)

Set your map data.  
*Setting new map data reset the current cache.

**PARAMETERS**
* ```world``` (table) - Your tilemap data. Keep it simple as much as you can.

**EXAMPLE**
```lua

local world = {
2 ,2 ,0 ,1 ,0,
0 ,0 ,1 ,2 ,1,
0 ,0 ,2 ,0 ,0,
0 ,2 ,2 ,2 ,2,
2 ,1 ,1 ,0 ,1
}

astar.set_map(world)

```

## astar.set_costs(`costs`)

Set costs for your walkable tiles on your `world` table. This table keys determines the walkable area. In this example only numbered "2" tiles are walkable.   

Table's sum must be the `astar.DIRECTION_FOUR` or `astar.DIRECTION_EIGHT`. In this example we want to move 8 direction. 

*Setting new cost data reset the current cache.

**EXAMPLE**

```lua

local costs = {
    [2] = {
        1.0, -- E
        1.0, -- N
        1.0, -- W
        1.0, -- S
        1.41, -- NE
        1.41, -- NW
        1.41, -- SW
        1.41 -- SE
    }
}

astar.set_costs(costs)

```

## astar.solve(`start_x`, `start_y`, `end_x`, `end_y`)

Solves the path.   

**PARAMETERS**

X, Y of the tile possition on array not the screen position.

* ```start_x``` (int) - Start tile X
* ```start_y``` (int) - Start tile Y
* ```end_x``` (int) - End tile X
* ```end_y``` (int) - End tile Y

**RETURN**

* ```result``` (enum) -

	**astar.SOLVED**: Path solved  
	**astar.NO_SOLUTION**: Can't find the path  
	**astar.START_END_SAME**: Start and End is the same 

* ```size``` (int) - Size of the path.
* ```total_cost``` (int) - Total cost of the path
* ```path``` (table) -  Table with x and y coordinates. First value is the given start point.

**EXAMPLE**

```lua

local start_x = 1
local start_y = 1
local end_x = 3
local end_y = 3

local result, size, total_cost, path = astar.solve(start_x, start_y, end_x, end_y)

if result == astar.SOLVED then
	print("SOLVED")
	for i, v in ipairs(path) do
		print("Tile: ", v.x .. "-" .. v.y)
	end
elseif result == astar.NO_SOLUTION then
	print("NO_SOLUTION")
elseif result == astar.START_END_SAME then
	print("START_END_SAME")
end

```

## astar.solve_near(`start_x`, `start_y`, `max_cost`)

Finds the neighbours according to given cost.   

**PARAMETERS**
* ```start_x``` (int) - Start tile X
* ```start_y``` (int) - Start tile Y
* ```max_cost``` (int) - Maximun cost for finding neighbours

**RETURN**

* ```near_result``` (enum) - 

	**astar.SOLVED**: Path solved  
	**astar.NO_SOLUTION**: Can't find the path  
	**astar.START_END_SAME**: Start and End is the same 

* ```near_size``` (enum) - Size of the found neighbours.
* ```nears``` (enum) - Table with x and y coordinates. First value is the given start point.  

**EXAMPLE**

```lua

local start_x = 1
local start_y = 1
local max_cost = 3.0 -- near

local near_result, near_size, nears = astar.solve_near(start_x, start_y, max_cost)

if near_result == astar.SOLVED then
	print("SOLVED")
	for i, v in ipairs(nears) do
		print("Tile: ", v.x .. "-" .. v.y)
	end
elseif near_result == astar.NO_SOLUTION then
	print("NO_SOLUTION")
elseif near_result == astar.START_END_SAME then
	print("START_END_SAME")
end

```


## astar.reset_cache()

If your state space is huge, occasionally call astar.reset_cache() to free unused memory.



## astar.get_at(`x`, `y`)

Returns the value from the map array by coordinates.

**PARAMETERS**

X, Y of the tile possition on array not the screen position.

* ```x``` (int) - Tile X
* ```y``` (int) - Tile Y


**RETURN**

* ```value``` (int)


**EXAMPLE**

```lua

local value = astar.get_at(1, 1)

if value > 0 then
	print("boo")
end

```


## astar.set_at(`x`, `y`, 'value')

Set your value to the map array by coordinates.
*Setting new data reset the current cache.

**PARAMETERS**

X, Y of the tile possition on array not the screen position.

* ```x``` (int) - Tile X
* ```y``` (int) - Tile Y
* ```value``` (int) 


**EXAMPLE**

```lua

astar.set_at(1, 1, 0)


```

---

## Games using A-Star


### Monkey Mart

![DAABBCC](/.github/monkey_mart.jpg?raw=true)

**Developer:** [Tiny Dobbins](http://tinydobbins.com/)  
**Release Date:** 8 Dec, 2022  
**Poki:** https://poki.com/en/g/monkey-mart
