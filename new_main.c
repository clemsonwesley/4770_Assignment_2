#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

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
#define GRID_SIZE (ROWS + 2) * (COLS + 2)
//room temperatures
#define FIRE_TEMP 300
#define MESH_TEMP 20
//# of iterations to calculate
#define ITERATIONS 1000

struct GridPoint
{
	float temperature;
	short canTempChange; //apparently bools are undefined ...
};

//utility methods
struct GridPoint* AllocateRoom();
void FreeRoom(struct GridPoint* room);
void ConfigureDataType();
void InitializeRoom(struct GridPoint* room);

void TransferWork(struct GridPoint* workSpace, int startPosition, int endPosition);
void CopyNewToOld(struct GridPoint* oldRoom, struct GridPoint* newRoom);
void CalculateNew(struct GridPoint* oldRoom, struct GridPoint* newRoom, int startPosition, int endPosition);
void PrintGridToFile(struct GridPoint* room);

int rank = 0;
int numTasks = 0;
MPI_Datatype gridPointType;

int main(int argc, char* argv[])
{
	int source = 0, dest, tag = 1;

	//initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

	//setup GridPoint w/ MPI
	ConfigureDataType();

	//at the start all grids will be the same
	struct GridPoint* newRoom = AllocateRoom();
	struct GridPoint* oldRoom = AllocateRoom();
	if(rank == 0) InitializeRoom(newRoom);
	MPI_Scatter(newRoom, (COLS + 2) * (ROWS + 2), gridPointType, newRoom, (COLS + 2) * (ROWS + 2), gridPointType, 0, MPI_COMM_WORLD);
	printf("room scattered\n");
	PrintGridToFile(newRoom);
	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	int i;

	for (i = 0; i < ITERATIONS; i++)
	{
		if (rank == 0) printf("Iteration %d\n", i);
		if(rank == 0) CopyNewToOld(oldRoom, newRoom);
		int startPosition = (int)roundf(blockSize * rank);
		int endPosition = (int)roundf(blockSize * (rank + 1));
		printf("Here: %d\n", rank);
		printf("process %d will start at %d and end at %d\n", rank, startPosition, endPosition);
		CalculateNew(oldRoom, newRoom, startPosition, endPosition);
		printf("process %d has finished calculating at iteration %d\n", rank, i);
		TransferWork(newRoom, startPosition, endPosition);
		//printf("process %d has finished transferring work at iteration %d\n", rank, i);
		//need to redo mpi stuff here
		MPI_Barrier(MPI_COMM_WORLD);
	}
	if (rank == 0) PrintGridToFile(newRoom);

	FreeRoom(newRoom);
	FreeRoom(oldRoom);

	MPI_Finalize();
}

void TransferWork(struct GridPoint* workSpace, int startPosition, int endPosition)
{
	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	//printf("Before the request\n");
	MPI_Request requests[numTasks + 1];
	//printf("after the request\n");
	MPI_Status statuses[numTasks + 1];
	//printf("after the stat\n");
	const int transferSize = (endPosition - startPosition) * (ROWS + 2);
	//converting this to a 1D array so it can be sent over MPI (hopefully)
	//struct GridPoint* gridStart = workSpace[startPosition];
	/*if (rank != numTasks - 1)
	{
		printf("Send: %d\n", rank);
		MPI_Send(workSpace, transferSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD);
	}
	if(rank != 0)
	{
		printf("First: %d\n", rank);
		const int recvSize = ((int)roundf(blockSize * (rank + 1)) - (int)roundf(blockSize * (rank))) * (ROWS + 2);
		MPI_Recv(&workSpace[endPosition-1], recvSize, gridPointType, rank +1, 1, MPI_COMM_WORLD,
		         &statuses[rank]);
	}*/
	/*if (rank != 0)
	{
		printf("reach rank!=0 send\n");
		MPI_Send(workSpace, transferSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD);
	}*/
	if(rank == 0){
		printf("here? %d\n", rank);
		const int recvSize = ((int)roundf(blockSize * (rank + 2)) - (int)roundf(blockSize * (rank + 1))) * (ROWS + 2);
		MPI_Recv(&workSpace[endPosition+1], recvSize, gridPointType, rank+1, 1, MPI_COMM_WORLD,
		         &statuses[rank]);
    MPI_Send(workSpace, transferSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD);
		printf("rank 0 breakthrough\n");

	  printf("deadlock here?\n");

	}
	else if (rank != numTasks - 1)
	{
		printf("rank != numTasks -1? %d, %d\n", rank, numTasks);
		MPI_Send(workSpace, transferSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD);
		printf("receiving size %d\n", rank);
		const int recvSize = ((int)roundf(blockSize * (rank + 2)) - (int)roundf(blockSize * (rank + 1))) * (ROWS + 2);
		printf("%d", rank);
		MPI_Recv(&workSpace[endPosition-1], recvSize, gridPointType, rank-1, 1, MPI_COMM_WORLD,
		         &statuses[rank]);

		MPI_Recv(&workSpace[endPosition-1], recvSize, gridPointType, rank+1, 1, MPI_COMM_WORLD,
		         &statuses[rank]);
		MPI_Send(workSpace, transferSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD);
	}

	else if(rank == numTasks - 1){

		printf("reached rank == numTasks -1 send: %d\n", rank);
		MPI_Send(workSpace, transferSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD);
		const int recvSize = ((int)roundf(blockSize * (rank + 2)) - (int)roundf(blockSize * (rank + 1))) * (ROWS + 2);
		MPI_Recv(&workSpace[endPosition+1], recvSize, gridPointType, rank-1, 1, MPI_COMM_WORLD,
						 &statuses[rank]);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	//MPI_Gather(workSpace, transferSize, gridPointType);
}


/*
* copy the new room to the old room
*/
void CopyNewToOld(struct GridPoint* oldRoom, struct GridPoint* newRoom)
{
	const int size = (COLS + 2) * (ROWS + 2);
	int i;
	for (i = 0; i < (COLS + 2) * (ROWS + 2); i++)
		oldRoom[i] = newRoom[i];
}

/*
* calculate the next temperature of the mesh
*/
void CalculateNew(struct GridPoint* oldRoom, struct GridPoint* newRoom, int startPosition, int endPosition)
{
	printf("calc new room start/n");
	const int cols = (COLS + 2);
	int i, j;
	for (i = startPosition; i < endPosition; i++)
	{
		for (j = 0; j < ROWS + 2; j++)
		{
			//if the temp can't change we're probably on a fire so skip the calculation
			if (oldRoom[i * cols + j].canTempChange == 0)
				continue;
			newRoom[i * cols + j].temperature = 0.25 * (oldRoom[(i - 1) * cols + j].temperature + oldRoom[(i + 1) * cols + j].
				temperature + oldRoom[i * cols + j + 1].temperature + oldRoom[i * cols + j - 1].temperature);
		}
	}
	printf("calc new room finished/n");
}

void PrintGridToFile(struct GridPoint* room)
{
	int i, j;
	/* Colors are list in order of intensity */
	char* colors[10] = {
		RED, ORANGE, YELLOW, LTGREEN, GREEN,
		LTBLUE, BLUE, DARKTEAL, BROWN, BLACK
	};

	/* The pnm filename is hard-coded.  */

	FILE* fp = fopen("c.pnm", "w");

	fprintf(fp, "P3\n%d %d\n15\n", ROWS, COLS);

	for (i = 1; i < COLS + 1; i++)
	{
		for (j = 1; j < ROWS + 1; j++)
		{
			struct GridPoint point = room[j * (ROWS + 2) + i];
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
	system("convert c.pnm c.gif"); /* png not supported on comp */
}

struct GridPoint* AllocateRoom()
{
	struct GridPoint* grid = malloc(sizeof(struct GridPoint*) * (COLS + 2) * (ROWS + 2));
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
	const int cols = (COLS + 2);
	int i, j;
	const int fireplace_start = (ROWS / 10) * 3 + 1; //start at 40%
	const int fireplace_end = (ROWS / 10) * 7 + 1; //end at 80%
	//initialize everything
	for (i = 0; i < COLS + 2; i++)
	{
		for (j = 0; j < ROWS + 2; j++)
		{
			room[i * cols + j].canTempChange = 1;
			room[i * cols + j].temperature = MESH_TEMP;
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
	MPI_Aint offsets[2], extent;
	//create description of the single float field
	offsets[0] = 0;
	oldTypes[0] = MPI_FLOAT;
	blockCounts[0] = 1;
	//create description of the single short field
	MPI_Type_extent(MPI_FLOAT, &extent);
	offsets[1] = 1 * extent;
	oldTypes[1] = MPI_SHORT;
	blockCounts[1] = 1;
	//define structured type and commit
	MPI_Type_struct(2, blockCounts, offsets, oldTypes, &gridPointType);
	MPI_Type_commit(&gridPointType);
	//end definition, phew...
}
