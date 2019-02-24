#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <mpi/mpi.h>

//colors for output file
#define WHITE "15 15 15 "
#define RED "15 00 00 "
#define ORANGE "15 05 00 "
#define YELLOW "15 10 00 "
#define LTGREEN "00 13 00 "
#define GREEN "05 10 00 "
#define LTBLUE "00 05 10 "
#define BLUE "00 00 10 "
#define DARKTEAL "00 05 05 "
#define BROWN "03 03 00 "
#define BLACK "00 00 00 "
//define grid
#define ROWS 1000
#define COLS 1000
#define GRID_SIZE ((ROWS + 2) * (COLS + 2))
//room temperatures
#define FIRE_TEMP 300
#define MESH_TEMP 20
//# of iterations to calculate
#define ITERATIONS 50000

struct GridPoint
{
	int id, canTempChange;
	float temperature;
};

//utility methods
struct GridPoint* AllocateRoom();
void FreeRoom(struct GridPoint* room);
void ConfigureDataType();
void InitializeRoom(struct GridPoint* room);

void TransferWork(struct GridPoint* workSpace, int startPosition, int endPosition);
void CopyNewToOld(struct GridPoint* oldRoom, const struct GridPoint* newRoom);
void CalculateNew(const struct GridPoint* oldRoom, struct GridPoint* newRoom, int startPosition, int endPosition);
void PrintGridToFile(const struct GridPoint* room, char* file_name);

int rank = 0;
int numTasks = 0;
MPI_Datatype gridPointType;

int main(int argc, char* argv[])
{
	//initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

	printf("starting process\n");


	//setup GridPoint w/ MPI
	ConfigureDataType();
	printf("data type configured\n");

	//at the start all grids will be the same
	struct GridPoint* newRoom = AllocateRoom();
	struct GridPoint* oldRoom = AllocateRoom();
	InitializeRoom(newRoom);
	printf("room generated\n");
	CopyNewToOld(oldRoom, newRoom);
	if (rank == 0) PrintGridToFile(newRoom, "init");
	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	int i;
	const int startPosition = (int)roundf(blockSize * rank);
	const int endPosition = (int)roundf(blockSize * (rank + 1));
	for (i = 0; i < ITERATIONS; i++)
	{
		if (rank == 0) printf("Iteration %d\n", i);
		CopyNewToOld(oldRoom, newRoom);
		CalculateNew(oldRoom, newRoom, startPosition, endPosition);
		TransferWork(newRoom, startPosition, endPosition);
		//need to redo mpi stuff here
		MPI_Barrier(MPI_COMM_WORLD);
	}

	if(rank == 0)
	{
		i = 1;
		for(; i < numTasks; i++)
		{
			//calculate the size that will be coming in for every element except this one
			const int upper_start_position = ((int)roundf(blockSize * i));
			const int upper_end_position = (int)roundf(blockSize * (i + 1));
			const int upper_transfer_size = (upper_end_position - upper_start_position) * (ROWS + 2);
			struct GridPoint* start_element = &newRoom[upper_start_position * (ROWS + 2)];
			printf("receiving %d, expecting size %d\n", i, upper_transfer_size);
			MPI_Status status;
			MPI_Recv(start_element, upper_transfer_size, gridPointType, i, 1, MPI_COMM_WORLD, &status);
		}
		PrintGridToFile(newRoom, "out");
	}
	if(rank != 0)
	{
		//send itself back proc 0
		const int transfer_size = (endPosition - startPosition) * (ROWS + 2);
		struct GridPoint * start_element = &newRoom[startPosition * (ROWS + 2)];
		MPI_Send(start_element, transfer_size, gridPointType, 0, 1, MPI_COMM_WORLD);
		printf("%d is transferring, sending %d elements\n", rank, transfer_size);
	}

	FreeRoom(newRoom);
	FreeRoom(oldRoom);

	MPI_Finalize();
}

void TransferWork(struct GridPoint* workSpace, int startPosition, int endPosition)
{
	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	const int transferSize = (endPosition - startPosition) * (ROWS + 2);
	//converting this to a 1D array so it can be sent over MPI (hopefully)
	//struct GridPoint* gridStart = workSpace[startPosition];
	if (rank != numTasks - 1)
	{
		MPI_Request request;
		struct GridPoint* start_element = &workSpace[(startPosition * (ROWS + 2))];
		MPI_Isend(start_element, transferSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD, &request);
	}
	if(rank != 0)
	{
		//calculate what will come in from the process below us
		const int lower_start_position = (int)roundf(blockSize * (rank - 1));
		const int lower_end_position = (int)roundf(blockSize * rank);
		const int lower_transfer_size = (lower_end_position - lower_start_position) * (ROWS + 2);
		//receive the lower data
		MPI_Status status;
		struct GridPoint* start_element = &workSpace[lower_start_position * (ROWS + 2)];
		MPI_Recv(start_element, lower_transfer_size, gridPointType, rank - 1, 1, MPI_COMM_WORLD, &status);

	} 
	if (rank != 0)
	{
		MPI_Request request;
		struct GridPoint* start_element = &workSpace[(startPosition * (ROWS + 2))];
		MPI_Isend(start_element, transferSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD, &request);
	}
	if (rank != numTasks - 1)
	{
		//calculate what will come in from the process above us
		const int upper_start_position = ((int)roundf(blockSize * (rank + 1)));
		const int upper_end_position = (int)roundf(blockSize * (rank + 2));
		const int upper_transfer_size = (upper_end_position - upper_start_position) * (ROWS + 2);
		//receive the upper data
		MPI_Status status;
		struct GridPoint* start_element = &workSpace[upper_start_position * (ROWS + 2)];
		MPI_Recv(start_element, upper_transfer_size, gridPointType, rank + 1, 1, MPI_COMM_WORLD, &status);
	}
}


/*
* copy the new room to the old room
*/
void CopyNewToOld(struct GridPoint* oldRoom, const struct GridPoint* newRoom)
{
	const int size = (COLS + 2) * (ROWS + 2);
	int i;
	for (i = 0; i < size; i++)
		oldRoom[i].temperature = newRoom[i].temperature;
}

/*
* calculate the next temperature of the mesh
*/
void CalculateNew(const struct GridPoint* oldRoom, struct GridPoint* newRoom, int startPosition, int endPosition)
{
	const int cols = (COLS + 2);
	int i, j;
	//can't remember if these two lines are important...
	if (rank != 0) startPosition -= 1; 
	if (rank != numTasks - 1) endPosition += 1;
	for (i = startPosition; i < endPosition; i++)
	{
		for (j = 0; j < ROWS + 2; j++)
		{
			//if the temp can't change we're probably on a fire so skip the calculation
			if (newRoom[i * cols + j].canTempChange == 0)
				continue;
			const float temperature = 0.25 * (oldRoom[(i - 1) * cols + j].temperature + oldRoom[(i + 1) * cols + j]
				.temperature + oldRoom[i * cols + j + 1].temperature + oldRoom[i * cols + j - 1].temperature);
			newRoom[i * cols + j].temperature = temperature;
		}
	}
}

void PrintGridToFile(const struct GridPoint* room, char* file_name)
{
	int i = 1, j = 1;
	char pnm_name[strlen(file_name) + 3];
	char gif_name[strlen(file_name) + 3];
	char command[100];
	sprintf(pnm_name, "%s.%s", file_name, "pnm");
	sprintf(gif_name, "%s.%s", file_name, "gif");
	sprintf(command, "convert %s %s", pnm_name, gif_name);

	FILE* fp = fopen(pnm_name, "w");  // NOLINT(android-cloexec-fopen)

	fprintf(fp, "P3\n%d %d\n15\n", ROWS, COLS);

	for (; i < COLS + 1; i++)
	{
		for (; j < ROWS + 1; j++)
		{
			const struct GridPoint point = room[j * (ROWS + 2) + i];
			if (point.temperature > 250)
			{
				fprintf(fp, "%s", RED);
			}
			else if (point.temperature > 180)
			{
				fprintf(fp, "%s", ORANGE);
			}
			else if (point.temperature > 120)
			{
				fprintf(fp, "%s", YELLOW);
			}
			else if (point.temperature > 80)
			{
				fprintf(fp, "%s", LTGREEN);
			}
			else if (point.temperature > 60)
			{
				fprintf(fp, "%s", GREEN);
			}
			else if (point.temperature > 50)
			{
				fprintf(fp, "%s", LTBLUE);
			}
			else if (point.temperature > 40)
			{
				fprintf(fp, "%s", BLUE);
			}
			else if (point.temperature > 30)
			{
				fprintf(fp, "%s", DARKTEAL);
			}
			else if (point.temperature > 20)
			{
				fprintf(fp, "%s", BROWN);
			}
			else
			{
				fprintf(fp, "%s", BLACK);
			}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	printf("room %s printed\n", file_name);
	system(command); /* png not supported on comp */
}

struct GridPoint* AllocateRoom()
{
	struct GridPoint* grid = malloc(sizeof(struct GridPoint) * GRID_SIZE);
	return grid;
}

void FreeRoom(struct GridPoint* room)
{
	free(room);
}

/*
* initialize the room to its default values
*/
void InitializeRoom(struct GridPoint* room)
{

	int i, j = 0;
	const int cols = (COLS + 2);
	const int fireplace_start = (ROWS / 10) * 3 + 1; //start at 40%
	const int fireplace_end = (ROWS / 10) * 7 + 1; //end at 80%
	//initialize everything
	for (i = 0; i < cols; i++)
	{
		for (; j < ROWS + 2; j++)
		{
			const int index = i * cols + j;
			room[index].canTempChange = 1;
			room[index].temperature = MESH_TEMP;
			room[index].id = index;
		}
	}
	//initialize the fireplace
	for (i = fireplace_start; i <= fireplace_end; i++)
	{
		room[i * cols + 1].canTempChange = 0;
		room[i * cols + 1].temperature = FIRE_TEMP;
	};
	//initialize horizontal walls
	for (i = 0; i < ROWS + 2; i++)
	{
		room[i * cols].canTempChange = 0;
		room[i * cols].temperature = MESH_TEMP;
		room[i * cols + (ROWS + 1)].canTempChange = 0;
		room[i * cols + (ROWS + 1)].temperature = MESH_TEMP;
	}
	//initialize the vertical walls
	for (i = 0; i < COLS + 2; i++)
	{
		room[i].canTempChange = 0;
		room[i].temperature = MESH_TEMP;
		room[(COLS + 1) * (ROWS + 2) + i].canTempChange = 0;
		room[(COLS + 1) * (ROWS + 2) + i].temperature = MESH_TEMP;
	}

}

void ConfigureDataType()
{
	//setup our struct
	MPI_Datatype oldTypes[2];
	int blockCounts[2];
	MPI_Aint offsets[2], intExtent, floatExtent;
	MPI_Type_extent(MPI_INT, &intExtent);
	MPI_Type_extent(MPI_FLOAT, &floatExtent);
	//create description of the two int fields
	offsets[0] = 0;
	oldTypes[0] = MPI_INT;
	blockCounts[0] = 2;
	//create description of the single float field
	offsets[1] = 2 * intExtent;
	oldTypes[1] = MPI_FLOAT;
	blockCounts[1] = 1;
	//define structured type and commit
	MPI_Type_struct(2, blockCounts, offsets, oldTypes, &gridPointType);
	MPI_Type_commit(&gridPointType);
	//end definition, phew...
}
