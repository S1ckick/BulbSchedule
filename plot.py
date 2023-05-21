f = open("build/test/sats.txt", 'r')

lines = f.readlines()
import numpy as np

import matplotlib.pyplot as plt

csfont = {'fontname' : 'PT Sans'}
plt.title('title',**csfont)

plt.figure(figsize=(20,30), dpi=300)
plt.xlabel("Timeline, sec", fontsize=20)
plt.ylabel("Satellite names", fontsize=20)


sats_names = []
sats_start = []
sats_end = []
sats = []

for line in lines:
    splitted = line.split(' ')
    if float(splitted[1]) <= 86400:
        #sats_names.append(splitted[0])

        x = np.linspace(int(float(splitted[1])), int(float(splitted[2])), int(float(splitted[2])) - int(float(splitted[1])))
        for i in x:
            sats.append(i)
            sats_names.append(splitted[0])
        #sats_start.append(float(splitted[1]))
        #sats_end.append(float(splitted[2]))


all_start = plt.scatter(sats,sats_names, c='green', s=5)
#all_end = plt.scatter(sats_end,sats_names, c='red', s=5)

plt.savefig('1.png',bbox_inches='tight')