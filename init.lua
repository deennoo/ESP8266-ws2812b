wifi.setmode(wifi.STATION)
wifi.sta.config("MELICO_MOGER", "1986434690")

dofile('telnet.lua')

tmr.alarm(1, 8000, 0, function() print(wifi.sta.getip()) dofile('milight.lua') end )
