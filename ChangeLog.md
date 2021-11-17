# v0.6.1
* add `map_area` to config ini, to restrict map drawing area
* `msg_position` is relative to the whole D2R window now
* add missing items to `D2RMH_data.ini` to avoid crash on filtering certain items
* hide overlay when D2R window is not foreground

# v0.6
* add edge line to map, and dim color for walkable area in default config
* add dropped item filter, with `show_items`, `msg_font_size`, `msg_bg_color` added to config ini
* add D2RMH_item.ini as item filter config, check comments inside for detail
* add `monster_color`, `unique_monster_color`, `npc_color`, `show_npc_name` to config ini
* add `msg_position` to config ini for message list position
* try a new way to locate current player in multiplayer games
* add reading installation path from registry as fallback
* change default config values (better for common use):
    * font_size: 12->14
    * position: 1->2
    * scale: 1.0->2.0
    * map_centered: 0->1
    * show_monster_name: 1->0
* fix bug that running D2RMH before D2R(windowed) causes overlay window shown out of window
* add an about dialog
* refactoring some remote mem reading codes to make it more readable
* refactoring string matching codes to improve performance

# v0.5.4
* show all players(including corpse) now
* add show_player_names to config ini
* show player's town portals and permanent portals now
* support 67005
* a potential crash and other bugs fix

# v0.5.3
* can detect monster aura type now
* always show NPCs
* add show_normal_monster to config ini
* add enchants display text settings to config ini, uses text color list from d2hackmap
* add FPS/VSync setting to config ini
* fix bug that `show_monster_immune` was not processed

# v0.5.2
* add monster resist(immune) display
* minor bug fixes and performance optimizations

# v0.5.1
* add nearby monsters(super-unique/boss/champion only) display
* fix shrine text stack issue, and remove opened shrines' title
* fix crash due to memory searching function changes

# v0.5
* support D2R 66878 update
* totally rewritten rendering engine, removes several dependencies(sokol, fontstash)
* add an exclusive-run check
* add a tray icon and remove program from taskbar
* optimize d2mapapi a bit
* show nearby shrines (you can disable it by set `show_objects=0` in D2RMH.ini)
* set default values for ini configurations so D2RMH can be run without D2RMH.ini (but you still need D2RMH_data.ini)
* add Caged Barbarian Quest to useful objects

# v0.4
* (#2) add 397 to useful_objects
* fixed path display between Rogue Encampment and Blood Moor, and similar paths
* reduce memory use of vertex buffers
* alpha and all color values can be set in configuration file now

# v0.3
* add various configurations in D2RMH.ini, check comments there
* fix Gidbinn guide line

# v0.2
* add display for Unique Chest, Well, neighbour map path
* fix display of correct taltomb entrance
* shorter line pointed to target, similar to legacy d2hackmap
* peformance tweaks to d2mapapi

# v0.1
* first release, with complete map revealing and quest/npc guides
