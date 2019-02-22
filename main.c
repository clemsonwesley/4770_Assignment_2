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
#define ITERATIONS 50000

struct GridPoint
{
	float temperature;
	short canTempChange; //apparently bools are undefined ...
};

//utility methods
struct GridPoint** AllocateRoom();
void FreeRoom(struct GridPoint** room);
void ConfigureDataType();
void InitializeRoom(struct GridPoint** room);

void TransferWork(struct GridPoint** workSpace, int startPosition, int endPosition);
void CopyNewToOld(struct GridPoint** oldRoom, struct GridPoint** newRoom);
void CalculateNew(struct GridPoint** oldRoom, struct GridPoint** newRoom, int startPosition, int endPosition);
void PrintGridToFile(struct GridPoint** room);

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
	struct GridPoint** newRoom = AllocateRoom();
	struct GridPoint** oldRoom = AllocateRoom();
	InitializeRoom(newRoom);

	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	int i;

	for (i = 0; i < ITERATIONS; i++)
	{
		if (rank == 0) printf("Iteration %d\n", i);
		CopyNewToOld(oldRoom, newRoom);
		int startPosition = (int)roundf(blockSize * rank);
		int endPosition = (int)roundf(blockSize * (rank + 1));
		//printf("process %d will start at %d and end at %d\n", rank, startPosition, endPosition);
		CalculateNew(oldRoom, newRoom, startPosition, endPosition);
		//printf("process %d has finished calculating at iteration %d\n", rank, i);
		TransferWork(newRoom, startPosition, endPosition);
		//printf("process %d has finished transferring work at iteration %d\n", rank, i);
		//need to redo mpi stuff here
		MPI_Barrier(MPI_COMM_WORLD);
	}
	PrintGridToFile(newRoom);

	//FreeRoom(newRoom);
	//FreeRoom(oldRoom);

	MPI_Finalize();
}

void TransferWork(struct GridPoint** workSpace, int startPosition, int endPosition)
{
	const float blockSize = (float)(COLS + 2) / (float)numTasks; //get size that this will be calculating
	MPI_Request requests[numTasks + 1];
	MPI_Status statuses[numTasks + 1];
	int transferSize = (endPosition - startPosition) * (ROWS + 2);
	//converting this to a 1D array so it can be sent over MPI (hopefully)
	struct GridPoint* gridStart = workSpace[startPosition];
	if (rank != numTasks - 1)
	{
		MPI_Send(gridStart, transferSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD);
	}
	if(rank != 0)
	{
		const int recvSize = ((int)roundf(blockSize * (rank)) - (int)roundf(blockSize * (rank - 1))) * (ROWS + 2);
		MPI_Recv((struct GridPoint*) (gridStart - recvSize), recvSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD, &statuses[rank - 1]);
	}
	if(rank != 0)
	{
		MPI_Send(gridStart, transferSize, gridPointType, rank - 1, 1, MPI_COMM_WORLD);
	}
	if(rank != numTasks - 1)
	{
		const int recvSize = ((int)roundf(blockSize * (rank + 2)) - (int)roundf(blockSize * (rank + 1))) * (ROWS + 2);
		MPI_Recv((struct GridPoint*) (gridStart + transferSize), recvSize, gridPointType, rank + 1, 1, MPI_COMM_WORLD, &statuses[rank]);
	}
}


/*
* copy the new room to the old room
*/
void CopyNewToOld(struct GridPoint** oldRoom, struct GridPoint** newRoom)
{
	int i, j;
	for (i = 0; i < ROWS + 2; i++)
		for (j = 0; j < COLS + 2; j++)
			oldRoom[i][j] = newRoom[i][j];
}

/*
* calculate the next temperature of the mesh
*/
void CalculateNew(struct GridPoint** oldRoom, struct GridPoint** newRoom, int startPosition, int endPosition)
{
	int i, j;
	for (i = startPosition; i < endPosition; i++)
	{
		for (j = 0; j < COLS + 2; j++)
		{
			//if the temp can't change we're probably on a fire so skip the calculation
			if (oldRoom[i][j].canTempChange == 0)
				continue;
			newRoom[i][j].temperature = 0.25 * (oldRoom[i - 1][j].temperature + oldRoom[i + 1][j].temperature + oldRoom[
				i][j - 1].temperature + oldRoom[i][j + 1].temperature);
		}
	}
}

void PrintGridToFile(struct GridPoint** room)
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
			struct GridPoint point = room[j][i];
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

struct GridPoint** AllocateRoom()
{
	struct GridPoint** grid = (struct GridPoint**)malloc(sizeof(struct GridPoint*) * (COLS + 2));
	int i;
	for (i = 0; i < COLS + 2; i++)
	{
		grid[i] = (struct GridPoint*)malloc(sizeof(struct GridPoint) * (ROWS + 2));
	}
	return grid;
}

void FreeRoom(struct GridPoint** room)
{
	int i;
	for (i = 0; i < COLS; i++)
		free(room[i]);
	free(room);
}

/*
* initialize the room to its default values
*/
void InitializeRoom(struct GridPoint** room)
{
	int i, j;
	const int fireplace_start = (ROWS / 10) * 3 + 1; //start at 40%
	const int fireplace_end = (ROWS / 10) * 7 + 1; //end at 80%
	//initialize everything
	for (i = 0; i < COLS + 2; i++)
	{
		for (j = 0; j < ROWS + 2; j++)
		{
			room[i][j].canTempChange = 1;
			room[i][j].temperature = MESH_TEMP;
		}
	}
	//initialize the fireplace
	for (i = fireplace_start; i <= fireplace_end; i++)
	{
		room[i][1].canTempChange = 0;
		room[i][1].temperature = FIRE_TEMP;
	};
	//initialize horizontal walls
	for (i = 0; i < ROWS + 2; i++)
	{
		room[i][0].canTempChange = 0;
		room[i][0].temperature = MESH_TEMP;
		room[i][ROWS + 1].canTempChange = 0;
		room[i][ROWS + 1].temperature = MESH_TEMP;
	}
	//initialize the vertical walls
	for (i = 0; i < COLS + 2; i++)
	{
		room[0][i].canTempChange = 0;
		room[0][i].temperature = MESH_TEMP;
		room[COLS + 1][i].canTempChange = 0;
		room[COLS + 1][i].temperature = MESH_TEMP;
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
