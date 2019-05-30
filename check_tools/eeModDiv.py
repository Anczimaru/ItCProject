### According to IEEE standard
def eEA(g,h,m):
    if h == 1 :
        return g

    r0 = m
    r1 = h % m
    s0 = 0
    s1 = g % m

    while r1 > 0:

        q = r0 // r1 # python operator, floor division
        r2 = (r0 - q * r1) % m
        s2 = (s0 - q * s1) % m

        r0 = r1
        r1 = r2
        s0 = s1
        s1 = s2

    if r0 == 1:
        return s0
    else:
        return r0




gs = "340E7BE2A280EB74E2BE61BADA745D97E8F7C300"
hs = "1E589A8595423412134FAA2DBDEC95C8D8675E58"
ps = "E95E4A5F737059DC60DFC7AD95B3D8139515620F"

#p = int(ps,16)
#g = int(gs,16)
#h = int(hs,16)

g = 9337555360448823227812410753177468631215558779020518084752618816205
h = 3949606626053374030787926457695139766118442946052311411513528958987
p = 22721622932454352787552537995910928073340732145944992304435472941311

out = eEA(g,h,p)

print(out)
