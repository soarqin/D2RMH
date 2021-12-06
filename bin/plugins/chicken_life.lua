function chicken_life()
    local player = get_player()
    if player and player.map ~= 1 and player.map ~= 40 and player.map ~= 75 and player.map ~= 103 and player.map ~= 109 then
        if player.stats[7] < player.stats[8] * 0.3 then
            kill_process()
        end
    end
end

register_plugin(500, chicken_life)
