function chicken_life()
    local stats = get_player_stats()
    if stats then
        if stats[7] < stats[8] * 0.3 then
            kill_process()
        end
    end
end

register_plugin(500, chicken_life)
