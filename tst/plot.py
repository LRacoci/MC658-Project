import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
import os

cmap = cm.get_cmap('hsv')

b = "./heuristicas"
tsts = os.listdir(b)
data = {}
for tst in tsts:
	print tst
	nums = os.listdir('/'.join([b,tst]))
	data[tst] = {}
	for num in nums:
		name = '/'.join([b,tst,num]) 
		print '\t',num, "-->", name

		with open(name, 'r') as dataFile:
			data[tst][num] = [line[:-1].split('\t') for line in dataFile]

		data[tst][num] = map(list, zip(*data[tst][num]))
'''
arrData = {tst: np.array([run for run in data[tst]])}
'''

print "{"
for tst in data:
	print '1:\t', tst, ':'
	for num in data[tst]:
		print '2:\t\t', num, ':'
		x,y = data[tst][num]
		print '3:\t\t\tx: ', x
		print "3:\t\t\ty: ", y
print "}"

fig = plt.figure()
for color, tst in enumerate(data):
	for num in data[tst]:
		ax = fig.add_subplot(111, label=tst+'-'+num)
		y,x = data[tst][num]
		ax.plot(x, y, c = cmap(float(color)/len(tsts)))


ax.set_xlabel("Iteracoes")
ax.set_ylabel("Custo")

ax.set_yscale("linear")
ax.set_xscale("linear")
ax.grid()

plt.show()
'''


'''