import random
import os
import subprocess

a = random.randint(100000000,999999999)
b = random.randint(100000000,999999999)
c = random.randint(100000000,999999999)

cmd = f"cd to-player && printf \'debug\n{a}\n{b}\n{c}\n\' | ./main > ../flag"
os.system(cmd)
flag = open('./flag','rt').read().split('\n')[-2]
print('Challenge initialized.')
print(flag)
os.system('rm -f ./flag')