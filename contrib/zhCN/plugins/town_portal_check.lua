local text = create_text_list("default")

local town_portal_check = function()
    -- game fully loaded?
    if not get_skill(0) then
        return
    end

    local quantity
    local portal_book = get_skill(220)
    if portal_book then
        quantity = portalbook.quantity
    else
        quantity = 0
    end
    if quantity < 3 then
        text:add(string.format("\x0B警告！回城卷轴数量不足: %d", quantity), 1500, 0)
    end
end
register_plugin(1000, town_portal_check)
