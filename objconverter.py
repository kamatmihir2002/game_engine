import sys
import numpy as np

with open(sys.argv[1], "r") as F:
    verts = []
    norms = []
    tex = []
    lines = F.readlines()
    idx = []
    madecopy = False
    for line in lines:
        ele = line.split()
        if "v" in line:
            if "vn" in line:
                norms.append([float(element) for element in ele[1:]])
            elif "vt" in line:
                tex.append([float(element) for element in ele[1:]])
            else:
                verts.append([float(element) for element in ele[1:]])
                
        elif "f" in line:
            if not madecopy:
                normscopy = verts.copy()
                vertscopy = verts.copy()
                uvcopy = verts.copy()
                madecopy = True
            
            for element in ele[1:]:
                v = (element.split("/"))
                idx.append(int(v[0]) - 1)
                normscopy[int(v[0]) - 1] = norms[int(v[2]) - 1].copy()
                uvcopy[int(v[0]) - 1] = tex[int(v[1]) - 1].copy()

npverts = np.array(verts)
npnorms = np.array(normscopy)
nptex = np.array(uvcopy)

print(len(verts))
print(len(normscopy))
print(len(idx))

with open(sys.argv[2], "wb") as F:
    print(npverts)
    print(npnorms)
    print(nptex)
    ia = len(verts).to_bytes(4, 'little')
    ib = len(normscopy).to_bytes(4, 'little')
    ic = len(idx).to_bytes(4, 'little')
    print(ic)
    F.write(ia)
    F.write("\n".encode())
    F.write(ib)
    F.write("\n".encode())
    F.write(ic)
    F.write("\n".encode())
    npa = npverts.astype(np.float32).tobytes()
    npb = npnorms.astype(np.float32).tobytes()
    npc = nptex.astype(np.float32).tobytes()
    npi = np.array(idx).astype(np.uint32).tobytes()
    
    print(type(npa))
    F.write(npa)
    F.write(npb)
    F.write(npc)
    F.write(npi)

                


        