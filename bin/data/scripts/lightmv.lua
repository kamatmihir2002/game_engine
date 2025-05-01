properties = {speed = 0.0, dt = 0.0, ct = 0.0}

function init(self)
    sceneprop(self).ct = 0.0
    sceneprop(self).dt = 0.0
    print(self)
end

function update(self, dt)
    props = sceneprop(self)
    props.ct = props.ct + dt;
    
    _internal_rotate(self, 0.001, 0.0, 0.0, 1.0)
end

