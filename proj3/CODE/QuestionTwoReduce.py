import sys
from collections import defaultdict

current_user = -1
current_movie = -1
current_user_movieCount = 0
genres = defaultdict(int)

global_max_user = -1
global_max_user_ratings = -1
global_max_user_genre = None
global_max_user_genre_count = -1

for line in sys.stdin:
    line = line.strip()
    userInfo = line.split('\t')
    if current_user != int(userInfo[0]):
        if(current_user_movieCount > global_max_user_ratings):
            global_max_user = current_user
            global_max_user_ratings = current_user_movieCount
            max_genre = ''
            max_genre_count = 0
            for value in genres:
                if(genres[value] > max_genre_count):
                    max_genre = value
                    max_genre_count = genres[value]
            global_max_user_genre = max_genre
            global_max_user_genre_count = max_genre_count
        current_user = int(userInfo[0])
        current_movie = int(userInfo[1])
        genres.clear()
        current_user_movieCount = 1
        genres[userInfo[2]] += 1
    else:
        if(int(userInfo[1]) != current_movie):
            current_user_movieCount += 1
            current_movie = int(userInfo[1])
        genres[userInfo[2]] += 1

#copy pasted from earlier b/c it doesn't get fired at the very end
if(current_user_movieCount > global_max_user_ratings):
    global_max_user = current_user
    global_max_user_ratings = current_user_movieCount
    max_genre = ''
    max_genre_count = 0
    for value in genres:
        if(genres[value] > max_genre_count):
            max_genre = value
            max_genre_count = genres[value]
    global_max_user_genre = max_genre
    global_max_user_genre_count = max_genre_count

print('Output for sample user identification')
print('{0} -- Total Rating Counts: {1} -- Most Rated Genre: {2} - {3}'
    .format(global_max_user, global_max_user_ratings, global_max_user_genre,
     global_max_user_genre_count))