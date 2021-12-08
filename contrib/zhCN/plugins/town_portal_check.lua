local text = create_text_list("default")

local town_portal_check = function()
    local skill = get_skill(220)
    if not skill or skill.quantity < 3 then
        text:add("\x0B警告！回城卷轴数量不足", 1500, 0)
    end
end
register_plugin(1000, town_portal_check)
