complex = require('complex')

a = complex(1.2, 3.4)
print('a:', a)
b = complex(2.3, 4.5)
print('b:', b)
print('a+b:', a+b)
print('a-b:', a-b)
print('a*b:', a*b)
print('a/b:', a/b)
print('a^b:', a^b)
print('a.real:', a.real)
print('a.imag:', a.imag)
print('a.norm:', a.norm)
print('a.normsq:', a.normsq)
print('a.arg:', a.arg)
print('a.conj:', a.conj)
print('a.norm1:', a.norm1)
print('complex.i:', complex.i)
print('a==b', a==b)
print('expi2pi(0.5)+1', complex.expi2pi(0.5)+1)
--[[
for x = 0,1,0.01 do
	z = complex.expi2pi(x)
	print(x, z)
end
]]
