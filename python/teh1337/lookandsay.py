#!/usr/bin/env python
def process(seq):
        ones_twos_threes = map(lambda x: 0, range(3))
        last = 0
        output = ""

        seq = map(int,list(seq))
        if not len(seq): return "1"
        
        for i in range(len(seq)):
                if i == 0: last = seq[i]
                if seq[i] == last:
                        ones_twos_threes[last - 1] += 1
                        last = seq[i]

                if seq[i] != last or i == len(seq)-1:
                        if seq[i] != last:
                                output += str(ones_twos_threes[last-1])+str(last)
                                ones_twos_threes = map(lambda x: 0, range(3))
                                last = seq[i]
                                ones_twos_threes[last - 1] += 1
                                if i == len(seq)-1:
                                        output += str(ones_twos_threes[last-1])+str(last)

                        elif seq[i] == last and i == len(seq)-1:
                                 output += str(ones_twos_threes[last-1])+str(last)
  
        return output


lines = 0
while lines < 2: lines = input("lines: ")

seq = ""
for a in range(lines):
        seq = process(seq)
        print seq
