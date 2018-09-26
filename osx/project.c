#include "/opt/raylib/src/raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define assert(expr) if(!(expr)) {printf("%s:%d %s() %s\n", __FILE__, __LINE__, __func__, #expr); __builtin_trap();}


bool left() {
    if (IsKeyDown(KEY_LEFT)) {
        return 1;
    } else {
        return 0;
    }
}
bool right() {
    if (IsKeyDown(KEY_RIGHT)) {
        return 1;
    } else {
        return 0;
    }
}

bool down() {
    if (IsKeyDown(KEY_DOWN)) {
        return 1;
    } else {
        return 0;
    }
}

bool downPressed() {
    if ( IsKeyPressed(KEY_DOWN)) {
        return 1;
    } else {
        return 0;
    }
}
bool up() {
    if (IsKeyDown(KEY_UP)) {
        return 1;
    } else {
        return 0;
    }
}
bool upPressed() {
    if (IsKeyPressed(KEY_UP)) {
        return 1;
    } else {
        return 0;
    }
}
bool waterPressed() {
    if (IsKeyPressed(KEY_W))  {
        return 1;
    } else {
        return 0;
    }
}

bool actionPressed() {
    if ( IsKeyPressed(85) ||IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))  {
        return 1;
    } else {
        return 0;
    }
}


#define SCREEN_WIDTH 1234
#define ASSETS "assets/" 
void LoadWaterData(void * water_a, size_t size) {
    char filename[40];
    strcpy(filename, ASSETS);
    FILE *f = fopen(strcat(filename, "water.data"), "r");
    printf("readng %s\n", filename );
    printf("filename %s", filename);
    if (f != NULL) {
        printf("successfully read %s\n", filename );
        long int f_len;
        fseek(f, 0, SEEK_END);
        f_len = ftell(f);
        assert(f_len <= size);
        fseek(f, 0, SEEK_SET);
        fread(water_a, f_len, 1, f);
        fclose(f);
    }
    else {
        printf("Couldn't read %s\n", filename);
    }
}
#define Plant_Max 10000

typedef struct {
    int x;
    int y;
    Color color;
} Plant;

typedef struct {
    Plant items[Plant_Max];
    int count;
} Plant_a;


typedef struct {
    void * data;
    int size;
    int occupied;
    int high_water_mark;
} Temporary_Storage;

void WritePlantData(Plant_a * plant_a, size_t size, Temporary_Storage * temp) {
    // :write plants.data
    {
        char filename[40];
        strcpy(filename, ASSETS);
        FILE *f = fopen(strcat(filename, "plants.data"), "w");
        if (f) {
            fwrite((void *)plant_a, size, 1, f);
            fclose(f);
        }
    }

    // Write text based file. 
    {
        // @Concerns: Platform layer has to be aware of data for this to work. 
        // @Todo: It would be good to store date, time, and some kind of unique user id.. 
        FILE *f = fopen("assets/data_v1.txt", "w");
        strcpy(temp->data, "001\nOccupied: %ld      format created: 9/14/2018 format: x y color.r color.g color.b color.a\n");
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
        {
            char line[1000];
			sprintf(line, "current: %d/%d/%d %d:%d:%d\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
			strcat(temp->data,line);
        }
        for (int i = 0; i < plant_a->count; ++i) {
            char line[1000];
            sprintf(line, "%d %d %u %u %u %u\n", 
                plant_a->items[i].x,
                plant_a->items[i].y,
                plant_a->items[i].color.r,
                plant_a->items[i].color.g,
                plant_a->items[i].color.b,
                plant_a->items[i].color.a
            );
            strcat(temp->data, line);
        }
		temp->occupied += strlen(temp->data);
        if (temp->occupied > temp->high_water_mark) temp->high_water_mark = temp->occupied;
		sprintf(temp->data, temp->data, temp->occupied);
        if (f) {
            fwrite(temp->data, strlen(temp->data)-1, 1, f);
            fclose(f);
        }
        temp->occupied = 0;
    }
}

// :read plants.data
void LoadPlantData(Plant_a * plant_a, size_t size, Temporary_Storage * temp ) {
    char filename[40];
    strcpy(filename, ASSETS);
    //FILE *f = fopen(strcat(filename, "plants.data"), "r");
    FILE *f = fopen(strcat(filename, "data_v1.txt"), "r");
    // Read first byte of file to see what version it is. 
    int version = 1;
    if (version == 0) {
        // Version 0
        printf("V0 Data - readng %s sizeof plant_a %zu\n", filename, size );
        if (f != NULL) {
            printf("successfully read %s\n", filename );
            long int f_len;
            fseek(f, 0, SEEK_END);
            f_len = ftell(f);
            assert(f_len <= size);
            fseek(f, 0, SEEK_SET);
            fread(plant_a, f_len, 1, f);
            fclose(f);
        }
        else {
            printf("Couldn't read %s\n", filename);
        }
    }
    if (version == 1) {
		printf("successfully read %s\n", filename );
		long int f_len;
		fseek(f, 0, SEEK_END);
		f_len = ftell(f);
		assert(f_len <= size);
		fseek(f, 0, SEEK_SET);
		fread(temp->data, f_len, 1, f);
		fclose(f);
		// @Todo: use temporary memory for malloc probably. 
		static char  * row;
		if (!row) row = malloc(1000);
		static char  * col;
		if (!col) col = malloc(100);
		static char  * end_str;
		if (!end_str) end_str = malloc(1000);
		row = strtok_r (temp->data,"\n", &end_str);
		printf ("%s\n",row);
		row = strtok_r (NULL,"\n", &end_str);
		printf ("%s\n",row);
		row = strtok_r (NULL,"\n", &end_str);
		printf ("%s\n",row);
		row = strtok_r (NULL,"\n", &end_str);
		while (row != NULL) {
			printf ("row %s\n",row);
			char * end_token;
			plant_a->items[plant_a->count].x = atoi(strtok_r(row, " ", &end_token));
			plant_a->items[plant_a->count].y = atoi(strtok_r(NULL, " ", &end_token));
			plant_a->items[plant_a->count].color.r = atoi(strtok_r(NULL, " ", &end_token));
			plant_a->items[plant_a->count].color.g = atoi(strtok_r(NULL, " ", &end_token));
			plant_a->items[plant_a->count].color.b = atoi(strtok_r(NULL, " ", &end_token));
			plant_a->items[plant_a->count].color.a = atoi(strtok_r(NULL, " ", &end_token));
			++plant_a->count;
			row = strtok_r (NULL,"\n", &end_str);
	    }
		printf("done reading\n");
		printf("count %d\n", plant_a->count);
	}

}
void WriteWaterData(void * plant_a, size_t size) {
    // :write plants.data
    char filename[40];
    strcpy(filename, ASSETS);
    FILE *f = fopen(strcat(filename, "water.data"), "w");
    if (f) {
        fwrite(plant_a, size, 1, f);
        fclose(f);
    }
}

#include "vibrant.c"


int main(void) {
    Temporary_Storage temp;
    temp.size = 100000;
    temp.data = malloc(100000);
    temp.high_water_mark = 0;
    temp.occupied = 0;

    game(&temp);
}
