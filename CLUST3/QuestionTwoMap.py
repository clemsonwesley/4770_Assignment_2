import sys
import csv

movieList = {}

with open("./movies.csv", mode = 'r') as inFile:
    reader = csv.reader(inFile)
    for row in reader:
        movieList[int(row[0])] = row[2]

for line in sys.stdin:
    line = line.strip()
    words = line.split(',')
    try:
        movieId = int(words[1])
        userId = int(words[0])
        for genre in movieList[movieId].split('|'):
            print('{0:06}\t{1:06}\t{2}'.format(userId, movieId, genre))
    except:
        continue