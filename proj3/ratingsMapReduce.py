#First MapReduce program

#*************************************************************#
#
# Program Description: Finds the mean, median, and standard dev
# movie genre using MapReduce
#
#*************************************************************#

#!/usr/bin/env python

import sys

genres = ["Action", "Adventure", "Animation", "Children's", "Comedy", "Crime", "Documentary", "Drama", "Fantasy", "Film-Noir", "Horror", "Musical", "Mystery", "Romance", "Sci-Fi", "Thriller", "War", "Western"]

for line in sys.stdin:
