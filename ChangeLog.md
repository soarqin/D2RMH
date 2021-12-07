# v0.9.1
* finished basic lua plugin support, with plugins: chicken_life and town_portal_check
* add skill selection popup(0x100) to panel masks
* fix the bug that `fps` not working while set to positive value
* fix wrong size fetched on fullscreen mode for multiple monitors
* fix nearby map exits detection

# v0.9.0 prerelease
* new transparency mechanism, now you can set each color with alpha channel, and are stack with global `alpha` setting
* separate d2mapapi out as a standalone project, and re-add by git-subrepo, with lots of tweaks
* D2RMH main program can be built in 64bit now, while using `d2mapapi_piped` as a child process for querying map data

# v0.8.1
* fix lines for some quest targets
* fix random color blocks outside of map area sometime

# v0.8
* (#38) multi-instance support
* (#66) fix issue that map layer not shown for non-expansion characters
* (#68) fix issue that other players not shown
* search for memory offsets in order to get expansion flag address, offsets from [MapAssist](https://github.com/OneXDeveloper/MapAssist/blob/9b5658760efa8e8e243a4927d25abd2c796a41df/Helpers/ProcessContext.cs#L115-L167)
* fix `panel_mask` entry in config is not correctly processed
* add waypoint panel to `panel_mask`
* show neighbour maps, with `neighbour_map_bounds` entry added to D2RMH.ini
* dynamic loading functions from ntdll.dll and build release binaries with mingw32 now, to minify risks of virus detection

# v0.7
* (#47, #59, #63) add adaptive size for `map_area`, and is set as default value, kinda resolves confusion caused by new mechanism of `map_area`
* (#50) add text panel support, with new entries `text_panel_pattern` and `text_panel_position` in D2RMH.ini
* (#58) fix bug that monster immunities is not shown when disable `show_monster_enchants`
* (#60) add simple function that play sound on item dropping
* (#61) items sold to merchants are not detected now
* (#62) neutral NPCs in town, mercenaries and summons are shown as NPC now
* show real names of mercenaries now
* much simpler and faster way to detect real TalTomb, and show TalTomb with Super Unique monster(Ancient Kaa the Soulless) now
* set D2R process finding interval to 5 seconds, and remove WinMM from dependencies
* restore tray icon on Windows Explorer restart
* add `Reload Config` to tray menu for quick reloading D2RMH.ini

# v0.6.2
* add `panel_mask` to config ini, which can hide map layer when panels are opened
* fix the way to test local player
* fix a logic bug that causes map layer not shown once D2R is closed and reopen
* use real size in objects.txt for drawing map objects and draw doors on map layer now
* (#46) config entries are changed and fixed, now it is able to display names, enchants and immunities for normal monsters:
  * remove `show_normal_monsters`, merged into `show_monsters`, which has 3 available values now
  * `show_monster_name` => `show_monster_names` and has 3 available values now
  * `show_monster_enchant` => `show_monster_enchants` and has 3 available values now
  * `show_monster_immune` => `show_monster_immunities` and has 3 available values now
  * old config entries are still accepted, but please migrate to new names if possible
* (gendata) can read D2R installation path from registry now

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
* add an About dialog
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
