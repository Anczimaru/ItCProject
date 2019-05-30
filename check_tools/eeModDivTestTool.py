## Help of Internet. Not our function.
def extended_gcd(aa, bb):
    lastremainder, remainder = abs(aa), abs(bb)
    x, lastx, y, lasty = 0, 1, 1, 0
    while remainder:
        lastremainder, (quotient, remainder) = remainder, divmod(lastremainder, remainder)
        x, lastx = lastx - quotient*x, x
        y, lasty = lasty - quotient*y, y
    return lastremainder, lastx * (-1 if aa < 0 else 1), lasty * (-1 if bb < 0 else 1)
def modinv(a, m):
	g, x, y = extended_gcd(a, m)
	if g != 1:
		raise ValueError
	return x % m

#256B
#gs = "8BD2AEB9CB7E57CB2C4B482FFC81B7AFB9DE27E1E3BD23C23A4453BD9ACE3262"
#hs = "547EF835C3DAC4FD97F8461A14611DC9C27745132DED8E545C1D54C72F046997"
#ps = "A9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5372"
#160B
#gs = "340E7BE2A280EB74E2BE61BADA745D97E8F7C300"
#hs = "1E589A8595423412134FAA2DBDEC95C8D8675E58"
#ps = "E95E4A5F737059DC60DFC7AD95B3D8139515620F"

#p = int(ps,16)
#g = int(gs,16)
#h = int(hs,16)

g = 9337555360448823227812410753177468631215558779020518084752618816205
h = 3949606626053374030787926457695139766118442946052311411513528958987
p = 22721622932454352787552537995910928073340732145944992304435472941311

out = (g * modinv(h,p)) % p

#print(p)
#print(g)
#print(h)
#print(len("76884956397045344220809746629001649093037950200943055203735601445031516197746"))
print(out)
