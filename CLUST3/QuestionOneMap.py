import sys
import csv
import json

movieList = {}

index = -1
## get data from the user file
with open("./movies.csv", mode = 'r') as inFile:
    reader = csv.reader(inFile)
    for row in reader:
        index += 1
        if(index == 0):
            continue
        movieList[int(row[0])] = row[2]

# print(movieList)

for line in sys.stdin:
    line = line.strip()
    words = line.split(',')
    try:
        movieId = int(words[1])
        rating = words[2]
        for genre in movieList[movieId].split('|'):
            print('{0}\t{1}'.format(genre, rating))
    except:
        continue