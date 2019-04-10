import sys
import csv
import json

movieList = {}

## get data from the user file
with open("SampleMovies.csv", mode = 'r') as inFile:
    reader = csv.reader(inFile)
    for row in reader:
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