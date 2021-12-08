local toggle_show = function()
    local conf = get_config()
    conf.show = conf.show + 1
    if conf.show > 2 then
        conf.show = 0
    end
end

local zoom_in = function()
    local conf = get_config()
    if conf.scale < 4.0 then
        conf.scale = conf.scale + 0.5
        if conf.scale > 4.0 then
            conf.scale = 4.0
        end
        flush_overlay()
    end
end

local zoom_out = function()
    local conf = get_config()
    if conf.scale > 1.0 then
        conf.scale = conf.scale - 0.5
        if conf.scale < 1.0 then
            conf.scale = 1.0
        end
        flush_overlay()
    end
end

register_hotkey('\\', toggle_show)
register_hotkey('OEM_PLUS', zoom_in)
register_hotkey('OEM_MINUS', zoom_out)
