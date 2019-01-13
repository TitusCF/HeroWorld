local dir = "bottomworld"

function apply_fix(name, path)
    local line = love.filesystem.lines(path)
    local newtext = {}

    repeat
        text = line()
        if not text then break end
        table.insert(newtext,text)
        table.insert(newtext,"\n")
        if string.find(text, "arch")==1 and string.find(text, "map")~=6 then
            table.insert(newtext, "unique 1\nis_buildable 1\n")
        end
    until false

    love.filesystem.write(name,table.concat(newtext))
end

local files = love.filesystem.getDirectoryItems(dir)
for k,file in ipairs(files) do
    local path = dir.."/"..file
    local info = love.filesystem.getInfo(path)
    if info.type=="file" then
        apply_fix(file, path)
    end
end