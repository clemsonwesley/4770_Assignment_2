#include <stdio.h>
#include <stdlib.h>

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
	short canTempChange; //apparently bools are undefined ...
	float temperature;
};

struct GridPoint** AllocateRoom();
void FreeRoom(struct GridPoint** room);
void InitializeRoom(struct GridPoint** room);
void CopyNewToOld(struct GridPoint** oldRoom, struct GridPoint** newRoom);
void CalculateNew(struct GridPoint** oldRoom, struct GridPoint** newRoom);
void PrintGridToFile(struct GridPoint** room);

int main(int argc, char* argv[])
{
	//add two to the grid width and height to create walls
        struct GridPoint** newRoom = AllocateRoom();
	struct GridPoint** oldRoom = AllocateRoom();
	InitializeRoom(newRoom);

	for (int i = 0; i < ITERATIONS; i++)
	{
		printf("beginning iteration %d\n", i);
		//sleep(1);
		CopyNewToOld(oldRoom, newRoom);
		CalculateNew(oldRoom, newRoom);
	}
	PrintGridToFile(newRoom);

	FreeRoom(newRoom);
	FreeRoom(oldRoom);
}

/*
* copy the new room to the old room
*/
void CopyNewToOld(struct GridPoint** oldRoom, struct GridPoint** newRoom)
{
	for (int i = 0; i < ROWS + 2; i++)
		for (int j = 0; j < COLS + 2; j++)
			oldRoom[i][j] = newRoom[i][j];
}

/*
* calculate the next tempature of the mesh
*/
void CalculateNew(struct GridPoint** oldRoom, struct GridPoint** newRoom)
{
	for (int i = 0; i < ROWS + 2; i++)
	{
		for (int j = 0; j < COLS + 2; j++)
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
	/* Colors are list in order of intensity */
	char* colors[10] = {
		RED, ORANGE, YELLOW, LTGREEN, GREEN,
		LTBLUE, BLUE, DARKTEAL, BROWN, BLACK
	};

	/* The pnm filename is hard-coded.  */

	FILE* fp = fopen("c.pnm", "w");

	fprintf(fp, "P3\n%d %d\n15\n", ROWS, COLS);

	for (int i = 1; i < COLS + 1; i++)
	{
		for (int j = 1; j < ROWS + 1; j++)
		{
			struct GridPoint point = room[j][i];
			//printf("%d:%d--%f\n", j, i, point.temperature);
			//usleep(1000);
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
	for (int i = 0; i < COLS + 2; i++)
	{
		grid[i] = (struct GridPoint*)malloc(sizeof(struct GridPoint) * (ROWS + 2));
	}
	return grid;
}

void FreeRoom(struct GridPoint** room)
{
	for (int i = 0; i < COLS; i++)
		free(room[i]);
	free(room);
}

/*
* initialize the room to its default values
*/
void InitializeRoom(struct GridPoint** room)
{
	const int fireplace_start = (ROWS / 10) * 3 + 1; //start at 40%
	const int fireplace_end = (ROWS / 10) * 7 + 1; //end at 80%
	printf("fireplace start: %d\n", fireplace_start);
	printf("fireplace end: %d\n", fireplace_end);
	//initialize everything
	for (int i = 0; i < COLS + 2; i++)
	{
		for (int j = 0; j < ROWS + 2; j++)
		{
			room[i][j].canTempChange = 1;
			room[i][j].temperature = MESH_TEMP;
		}
	}
	//initialize the fireplace
	for (int i = fireplace_start; i <= fireplace_end; i++)
	{
		room[i][1].canTempChange = 0;
		room[i][1].temperature = FIRE_TEMP;
	};
	//initialize horizontal walls
	for (int i = 0; i < ROWS + 2; i++)
	{
		room[i][0].canTempChange = 0;
		room[i][0].temperature = MESH_TEMP;
		room[i][ROWS + 1].canTempChange = 0;
		room[i][ROWS + 1].temperature = MESH_TEMP;
	}
	//initialize the vertical walls
	for (int i = 0; i < COLS + 2; i++)
	{
		room[0][i].canTempChange = 0;
		room[0][i].temperature = MESH_TEMP;
		room[COLS + 1][i].canTempChange = 0;
		room[COLS + 1][i].temperature = MESH_TEMP;
	}
}
