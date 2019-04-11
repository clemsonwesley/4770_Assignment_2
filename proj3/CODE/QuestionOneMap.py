import sys
import csv
import json

movieList = {}
genreList = {}

## get data from the user file
with open("SampleMovies.csv", mode = 'r') as inFile:
    reader = csv.reader(inFile)
    for row in reader:
        movieList[int(row[0])] = row[2]

with open("SampleGenres.csv", mode = 'r') as otherFile:
    reader = csv.reader(otherFile)
    for row in reader:
        genreList[int(row[0])] = row[2]

#print(genreList)

for line in sys.stdin:
    line = line.strip()
   # print(line)    
    words = line.split(',')
    print
    try:
        movieId = int(words[1])
        rating = words[2]
        for genre in genreList[movieId].split('|'):
            print('{0}\t{1}'.format(genre, rating))
    except:
        continue
