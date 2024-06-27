![A*](/.github/defoldheader.png?raw=true)

This is a pathfinder and A* solver (astar or a-star) native extension for the [Defold Engine](https://www.defold.com/), built using [MicroPather](https://github.com/leethomason/MicroPather).  

## Installation

You can use the A* extension in your own project by adding this project as a [Defold library dependency](https://defold.com/manuals/libraries/#setting-up-library-dependencies).  
Open your `game.project` file, select  `Project ` and add a  `Dependencies ` field:

>https://github.com/selimanac/defold-astar/archive/master.zip

or you can add stable versions from [releases](https://github.com/selimanac/defold-astar/releases).  

## Examples

New examples are on the way.

Old examples: https://github.com/selimanac/defold-astar-examples


## Forum

https://forum.defold.com/t/a-native-extension/

## Toss a Coin to Your Witcher
If you find my [Defold Extensions](https://github.com/selimanac) useful for your projects, please consider [supporting](https://github.com/sponsors/selimanac) it.  
I'd love to hear about your projects! Please share your released projects that use my native extensions. It would be very motivating for me.


## Release Notes

### 1.1.0

This is a major release. Consider it a Beta version. New examples on the way.  
Feel free to open an [issue](https://github.com/selimanac/defold-astar/issues) if you encounter any problems.


- Now support odd-r, even-r, odd-q and even-q [hexagonal grids](https://www.redblobgames.com/grids/hexagons/#coordinates-offset). 
- Entities: You can now target entities like potions, chests, or enemies, which were not possible to solve on the map before. You can toggle `astar.use_entities()` before calling the `astar.solve()` and `astar.solve_near()` methods. 
- Partial code refactoring(nearly %30) for better readability, performance improvement(minimal) and Defold compatibility. I'll continue to improve it. Almost everything is now preallocated. You may hit the limits. If so, feel free to open an [issue](https://github.com/selimanac/defold-astar/issues)
- `astar.print_map()` added. You can print the latest map state for debug purposes.
- `astar.solve()` and `astar.solve_near()` now also returns tile content ID.
- `astar.map_vflip()` and `astar.map_hflip()` added. You can now flip the map.
- `astar.reset()` added. It cleans up everything. 
- Optional vertically flip map added to astar.setup(). 
- API Documentation updated.

**!! BREAKING CHANGE !!** 

The default value for tables and tile positions starts at 1. For backward compatibility, you can simply toggle it in `astar.setup()` or by using `astar.use_zero(true)`.


1.0.3  

- No changes. This is the final release before v1.1.

1.0.2  

- astar.set_at and astar.get_at added.
- astar.toogle_zero added. 

1.0.1  

Small bug fixes. [More details](https://forum.defold.com/t/a-native-extension/60405/9?u=selimanac) 

1.0  
initial
 
---

# API

## astar.setup(map_width, map_height, direction, allocate, typical_adjacent, [cache], [use_zero], [map_vflip] )

Initial setup. You have to setup the astar before calling any other methods.   

**PARAMETERS**. 

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

* ```cache``` (bool)[optional] - Turn on path caching. Uses more memory (yet again) but at a huge speed advantage if you may call the pather with the same path or sub-path, which is common for pathing over maps in games.  Default is `true`

*  ```use_zero``` (bool)[optional] - Toggle start index 0 or 1 for tables and tile positions. Also you can set it by call `astar.use_zero()`.  Default is `false`

* ```map_vflip``` (bool)[optional] - Flips the map vertically. This doesn't flip the coordinates. Also you can set it by call `astar.map_vflip()`. Default is `false`

**EXAMPLE**

```lua

local map_width = 5
local map_height = 4
local direction = astar.DIRECTION_EIGHT
local allocate = map_width * map_height
local typical_adjacent = 8
local cache = true     -- Optional. Default is true
local use_zero = false -- ptional. Default is false = 1 based
local flip_map = false -- Optional. Default is false

astar.setup(map_width, map_height, direction, allocate, typical_adjacent, cache, use_zero, flip_map)

```


## astar.use_zero(toggle)

Toggle start index 0 or 1 for tables and tile positions.

If set to `false`, [astar.solve](#astarsolvestart_x-start_y-end_x-end_y), [astar.solve_near](#astarsolve_nearstart_x-start_y-max_cost), [astar.get_at](#astarget_atx-y), [astar.set_at](#astarset_atx-y-value) methods expect positions start with 1 and returns table indexes from **1**.   

Default is `false` = 1

**PARAMETERS**

* ```toggle``` (bool) - true/false

**EXAMPLE**

```lua

astar.use_zero(false)


```




## astar.set_map_type(type)

You can set different type of map coordinates. Default is `astar.GRID_CLASSIC`.

More information about hexagonal grid offset coordinates systems: [Red Blob Games](https://www.redblobgames.com/grids/hexagons/#coordinates-offset)

**PARAMETERS**

* ```type``` (enum) - Type of the map coordinates.

	**astar.GRID_CLASSIC** : classic grid.  
	**astar.HEX_ODDR** : odd-r hexagonal grid.  
	**astar.HEX_EVENR** : even-r hexagonal grid.  
	**astar.HEX_ODDQ** : odd-q hexagonal grid.  
	**astar.HEX_EVENQ** : even-q hexagonal grid.  
	

**EXAMPLE**

```lua

astar.set_map_type(astar.HEX_ODDR)

```

## astar.set_map(world)

Set your map data.  
0.0 or 1.1 is top-left.  

*Setting new map data reset the current cache.  

**PARAMETERS** 

* ```world``` (table) - Your tilemap data. Keep it simple as much as you can.

**EXAMPLE**  

```lua

local world = {
	2, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	1, 0, 0, 0, 2,
}

astar.set_map(world)

```

## astar.map_vflip()

Every time you call this function, it flips the map vertically. This does not flip the coordinates

*Flipping the map reset the current cache.  


**EXAMPLE**  

```lua

local world = {
	2, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	1, 0, 0, 0, 2,
}

astar.set_map(world)
astar.map_vflip()
astar.print_map()
```


## astar.map_hflip()

Every time you call this function, it flips the map horizontally. This does not flip the coordinates

*Flipping the map reset the current cache.  


**EXAMPLE**  

```lua

local world = {
	2, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	1, 0, 0, 0, 2,
}

astar.set_map(world)
astar.map_hflip()
astar.print_map()
```




## astar.set_costs(costs)

Set costs for your passable tiles on your `world` table. This table keys determines the passable area. In this example only numbered "0" tiles are passable.   

Table's sum must be the `astar.DIRECTION_FOUR` (4) or `astar.DIRECTION_EIGHT` (8). In this example we want to move 8 direction. 

*Setting new cost data reset the current cache.

**PARAMETERS**. 

* ```costs``` (table) - Your tile cost data.  

**EXAMPLE**

```lua

-- For astar.DIRECTION_EIGHT
local costs = {
    [0] = {
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
-- OR
-- For astar.DIRECTION_FOUR
local costs = {
    [0] = {
        1.0, -- E
        1.0, -- N
        1.0, -- W
        1.0 -- S
    }
}

astar.set_costs(costs)


```

## astar.set_entities(entities)

Entities are usally not passible items like enemies or chests. You might want to target those tiles but also want to keep them not passable. You can set those entity ids as table. Your world table should contain those entities. 

You have to toggle `astar.get_entities()` before calling `astar.solve()` or `astar.solve_near()`. 


**PARAMETERS**  

* ```entities``` (table) - Your entities data.


**EXAMPLE**  

```lua

-- IDs of the entities
local entities = {
	1, 2
}

astar.set_entities(entities)

```


## astar.use_entities(toogle).

If you want to include your entities to your solve operations, you have to set it true before calling `astar.solve()` or `astar.solve_near()`.   

If set the `true`:   

`astar.solve()` only includes the end/last target tile to the path result. Other entities along the path stays as not passable.

`astar.solve_near()` includes all entities from entities table.

*When you call the `astar.use_entities()`, the current cache will be reset.

**PARAMETERS** 

* ```toogle``` (boolean) - Toogle true or false. Default is false


**EXAMPLE**

```lua

-- IDs of the entities
local entities = {
	1, 2
}

astar.set_entities(entities)
astar.use_entities(true)

```

## astar.get_at(x, y)

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

print(value)

```


## astar.set_at(`x`, `y`, `value`)

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


## astar.print_map()

Prints the map state for debug purposes.

**EXAMPLE**

```lua

astar.print_map()

```


```text

cols/x:
1	2	3	4	5	
-	-	-	-	-	
2	0	0	0	0		- row/y: 1
0	0	0	0	0		- row/y: 2
0	0	1	0	0		- row/y: 3
1	0	0	0	2		- row/y: 4

```

## astar.solve(start_x, start_y, end_x, end_y)

Solves the path.   

**PARAMETERS**

X, Y of the tile possition on array not the screen position.

* ```start_x``` (int) - Start tile X
* ```start_y``` (int) - Start tile Y
* ```end_x``` (int) - End tile X
* ```end_y``` (int) - End tile Y

**RETURN**

* ```status``` (enum) -

	**astar.SOLVED**: Path solved  
	**astar.NO_SOLUTION**: Can't find the path  
	**astar.START_END_SAME**: Start and End is the same 

* ```size``` (int) - Size of the path.
* ```total_cost``` (int) - Total cost of the path
* ```path``` (table) -  Table with x, y coordinates and tile ID. First value is the given start point.

**EXAMPLE**

```lua

local start_x = 1
local start_y = 1
local end_x = 3
local end_y = 3

local status, size, total_cost, path = astar.solve(start_x, start_y, end_x, end_y)

if status == astar.SOLVED then
	print("SOLVED")
	print("Path Size", size)
	print("Total Cost:", total_cost)

	for _, tile in ipairs(path) do
		print("x:", tile.x, "y: ", tile.y, "tile ID: ", tile.id)
	end
elseif status == astar.NO_SOLUTION then
	print("NO_SOLUTION")
elseif status == astar.START_END_SAME then
	print("START_END_SAME")
end

```

## astar.solve_near(start_x, start_y, max_cost)

Finds the neighbours according to given cost.   

**PARAMETERS** 

* ```start_x``` (int) - Start tile X
* ```start_y``` (int) - Start tile Y
* ```max_cost``` (float) - Maximun cost for finding neighbours

**RETURN**

* ```near_status``` (enum) - 

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

local near_status, near_size, nears = astar.solve_near(start_x, start_y, max_cost)

if near_status == astar.SOLVED then
	print("NEAR SOLVED")
	print("Near Size:", near_size)
	for _, tile in ipairs(nears) do
		print("x:", tile.x, "y: ", tile.y, "tile ID: ", tile.id)
	end
elseif near_status == astar.NO_SOLUTION then
	print("NO_SOLUTION")
elseif near_status == astar.START_END_SAME then
	print("START_END_SAME")
end

```


## astar.reset_cache()

If your state space is huge, occasionally call astar.reset_cache() to free unused memory.

**EXAMPLE**

```lua

astar.reset_cache()

```

## astar.reset()

This method removes everything releated to astar library and frees the memory. It may be used when the player completes/exits the level/game.

**EXAMPLE**

```lua

astar.reset()

```



---

## Games using A-Star


### Monkey Mart

![Monkey Mart](/.github/monkey_mart.jpg?raw=true)

**Developer:** [Tiny Dobbins](http://tinydobbins.com/)  
**Release Date:** 8 Dec, 2022  
**Poki:** https://poki.com/en/g/monkey-mart  
**Google Play:** https://play.google.com/store/apps/details?id=com.tinydobbins.monkeymart   
**App Store:** https://apps.apple.com/tr/app/monkey-mart/id6480208265  

### Xfire

![Xfire](/.github/xfire.png?raw=true)

**Developer:** [171Dev](https://www.171dev.uk)  
**Release Date:** 28 May, 2024  
**itch.io:** https://benjames171.itch.io/xfire  
**Source:** https://github.com/benjames-171/xfire 
