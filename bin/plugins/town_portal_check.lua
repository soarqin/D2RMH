local text = create_text_list("town_portal")
text.x = 0.2
text.y = 0.75
text.align = 0

function town_portal_check()
    local skill = get_skill(220)
    if not skill or skill.quantity < 3 then
        text:clear()
        text:add("\x0BWARNING! Town portal scrolls quantity low", 2000, 0)
    end
end
register_plugin(1000, town_portal_check)
