
a = 10
b = 5
c = a + b
d = a * b

x = a * b + a + b * a + b + c + c * d

#Check if with many statements inside
if x <= 100:
    y = 5 + 4 * 3 
    resA = 1
    resB = y
else:
    y = 10 + 2 + 4 
    resA = 0
    resB = y

if b < 6:
    t1 = 3
    t2 = 8
    resC = t1 + t2
else:
    t1 = 12
    t2 = 9
    resC = t1 + t2

print("resA =", resA)
print("resB =", resB)
print("resC =", resC)
