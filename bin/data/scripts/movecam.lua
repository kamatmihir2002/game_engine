properties = {speed = 0.0, dt = 0.0, ct = 0.0}

function init(self)
    
    sceneprop(self).ct = 0.0
end

function update(self, dt)
    props = sceneprop(self)
    
    _internal_translate(self, -0.002, 0.0, 0.0)
end

