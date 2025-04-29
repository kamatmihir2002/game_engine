properties = {speed = 0.0, dt = 0.0, ct = 0.0}

function init(self)
end

function update(self, dt)
    props = sceneprop(self)
    _internal_rotate(self, 0.0001, 0.0, 1.0, 0.0)
end

