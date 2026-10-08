-- launcher.lua — abre una ventana nueva cada vez que se pulsa F2
-- (el hotkey lo registra el core; aquí solo creamos ventanas de ejemplo)
local n = 0

function on_start()
    wm.log("launcher listo")
end

function on_tick()
    -- demostración: crea una ventana con F2 detectado vía título cambiante
    -- el core no expone hotkeys aún; este plugin crea ventanas programáticas
end

function open_demo()
    n = n + 1
    local id = wm.create_window("Lua demo " .. n, 220, 120)
    if id >= 0 then
        wm.notify("ventana #" .. id .. " creada desde Lua")
    end
end
