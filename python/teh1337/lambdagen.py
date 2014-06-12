import random
print "".join(map(chr,map(lambda x: random.randint(33,127),range(input('len: ')))))
