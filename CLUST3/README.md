# Command Arguments

## Question One Command Arguments

### Running The Application

#### Pulling The movies.csv file

To run the application you must first pull the movies.csv file in to the working directory that you plan to use to run the application, to do this use the command:

`hdfs dfs -cat /repository/movielens/movies.csv | sed 1d > movies.csv`

#### Running the Question One Mapreduce application

Once the file has been copied locally you can run the program, to do this use the command:

`hdfs dfs -cat /repository/movielens/ratings.csv | sed 1d | python3 QuestionOneMap.py | sort | python3 QuestionOneReduce.py [> optional outfile]`

##### Question One Note

you may need to change the name of "movies" file in QuestionOneReduce.py, currently it is set to use "SampleMovies.csv" as input, however it can be changed depending on what you wish to name your copy of the movies.csv file. The sample for pulling uses the file name that is already embedded in the file

## Question Two Command Arguments

### Running the Application

#### Prerequisites

Similarly to Question One, you must have the "movies.csv" file copied to the current working directory. To do this see the section "Pulling the movies.csv file"

#### Running the Question Two MapReduce

Once you have met the prerequisites you can run the program using:

`hdfs dfs -cat /repository/movielens/ratings.csv | sed 1d | python3 QuestionTwoMap.py | sort | python3 QuestionTwoReduce.py [> optional outfile]`

##### Question Two Note

Once again, you may have to change the name of the movies.csv file reference whatever you chose to call "movies.csv" for more details on this. The instructions for pulling this file use the name that the code references, so it is recommended that you use the same copy script