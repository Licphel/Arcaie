local printer = {}

function printer:dop(str)
    print(str)
end

Shared = {}
Shared.a = "a"

return printer
