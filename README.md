![ModPlayer](https://github.com/selimanac/defold-modplayer/blob/master/assets/defoldheader.png?raw=true)

This is a path finder and A* solver (astar or a-star) native extension for [Defold Engine](https://www.defold.com/) build on [MicroPather](https://github.com/leethomason/MicroPather). 

**Caution:** This extension is not battle tested yet and you should consider it as alpha release. It may contain bugs.

## Installation

You can use the A* extension in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/).  
Open your game.project file and in the dependencies field under project add:

>https://github.com/selimanac/defold-astar/archive/master.zip


## API

#### player.build_path(full_path:string)



Only required when developing on Defold Editor. `<FULL_PATH>` ([absolute path](https://en.wikipedia.org/wiki/Path_(computing))) is the full path of your project folder/directory.  
Don't forget to add **trailing `/`**.   


```lua
player.build_path("<FULL_PATH>/res/common/assets/") -- Set build path when working on Editor only s
```