import sys
import csv

genreList = {}

with open("SampleGenres.csv", mode = 'r') as otherFile:
    reader = csv.reader(otherFile)
    for row in reader:
        genreList[int(row[0])] = row[2]

print(genreList)
