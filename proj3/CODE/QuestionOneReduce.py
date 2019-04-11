import sys
from statistics import mean
from statistics import stdev
from statistics import median
from collections import defaultdict
from functools import reduce

current_genre = None
ratings = []

results = defaultdict(list)
#results = {}

for line in sys.stdin:
    line = line.strip()
    #print(line)
    ratingInfo = line.split("\t")
    #print('Values: {0}\t{1}\t{2}\n'.format(current_genre, current_rating_sum, current_rating_count))
    if current_genre != ratingInfo[0]:
       #print("hey")
        if(current_genre != None):
            avg = mean(ratings)
            stdDev = float
            # we know we have one because we got here in the first place
            if(len(ratings) < 2): stdDev = ratings[0]
            else: stdDev = stdev(ratings)
            med = median(ratings)
            results[current_genre] = [avg, med, stdDev]
        current_genre = ratingInfo[0]
        ratings.clear()
        ratings.append(float(ratingInfo[1]))

    else:
        ratings.append(float(ratingInfo[1]))

print('\nOutput for sample mean caluclation:')
for element in results:
    print('{0}\t\t{1}'.format(element, results[element][0]))
print('\nOutput for sample median caluclation:')
for element in results:
    print('{0}\t\t{1}'.format(element, results[element][1]))
print('\nOutput for sample standard deviation caluclation:')
for element in results:
    print('{0}\t\t{1}'.format(element, results[element][2]))
