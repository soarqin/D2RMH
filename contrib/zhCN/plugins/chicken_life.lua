local text = create_text_list("default")

local chicken_life = function()
    local player = get_player()
    if player and player.map ~= 1 and player.map ~= 40 and player.map ~= 75 and player.map ~= 103 and player.map ~= 109 then
        if player.stats[7] < player.stats[8] * 0.3 then
            kill_process()
        end
    end
end

function chicken_life_toggle(on)
    if on then
        text:add('生命安全线保护功能已开启  开关热键: \'Ctrl+/\'', 5000, -1)
    else
        text:add('生命安全线保护功能已关闭  开关热键: \'Ctrl+/\'', 5000, -1)
    end
end

register_plugin("Ctrl+/", false, 250, chicken_life, chicken_life_toggle)
