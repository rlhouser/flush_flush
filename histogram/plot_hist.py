import random
import numpy
from matplotlib import pyplot
import os

attacks = {'ff': 'Flush+flush', 'fr': 'Flush+reload'}

indirs = ['ff', 'fr', 'pp', 'ppc']
for indir in indirs:
	infile = indir + '/histogram.csv'
	print(infile)
	if not os.path.isfile(infile):
		continue
	hits = []
	misses = []

	with open(infile) as reader:
		for line in reader:
			l = line.strip().split('\t')
			if len(l) < 2:
				continue
			hits += [int(l[0])]*int(l[1])
			misses += [int(l[0])]*int(l[2])

	bins = numpy.linspace(min(hits), max(misses), 100)
	pyplot.hist(hits, bins, alpha=0.5, label='hits')
	pyplot.hist(misses, bins, alpha=0.5, label='misses')
	pyplot.legend(loc='upper right')
	pyplot.title(attacks[indir])
	pyplot.savefig(indir + '/histogram.png')
	pyplot.close()