#define ANDROID_FOPEN_H
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h> 
//#include "/opt/raylib/src/utils.h"

#include <sys/prctl.h> 
#include <android/log.h>

#include "/opt/raylib/src/raylib.h"
//#include <assert.h>
// F/libc    (12650): project.c:104: FileOpen: assertion "false" failed
#define assert(expr) if(!(expr)) { printf("%s:%d %s() %s", __FILE__, __LINE__, __func__, #expr);__builtin_trap(); }
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
FILE *android_fopen(const char *fileName, const char *mode);    // Replacement for fopen()


#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "VIBRANT", __VA_ARGS__);
bool left() {
    if (IsKeyDown(GAMEPAD_ANDROID_DPAD_LEFT) || IsKeyDown(KEY_LEFT)) {
        return 1;
    } else {
        return 0;
    }
}
bool right() {
    if (IsKeyDown(GAMEPAD_ANDROID_DPAD_RIGHT) || IsKeyDown(KEY_RIGHT)) {
        return 1;
    } else {
        return 0;
    }
}

bool down() {
    if (IsKeyDown(GAMEPAD_ANDROID_DPAD_DOWN) || IsKeyDown(KEY_DOWN)) {
        return 1;
    } else {
        return 0;
    }
}

bool downPressed() {
    if (IsKeyPressed(GAMEPAD_ANDROID_DPAD_DOWN) || IsKeyPressed(KEY_DOWN)) {
        return 1;
    } else {
        return 0;
    }
}
bool up() {
    if (IsKeyDown(GAMEPAD_ANDROID_DPAD_UP) || IsKeyDown(KEY_UP)) {
        return 1;
    } else {
        return 0;
    }
}
bool upPressed() {
    if (IsKeyPressed(GAMEPAD_ANDROID_DPAD_UP) || IsKeyPressed(KEY_UP)) {
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
    if (GetTouchPointsCount() > 0 || IsKeyPressed(85) || IsKeyPressed(GAMEPAD_ANDROID_BUTTON_B) || IsKeyPressed(KEY_ENTER))  {
        return 1;
    } else {
        return 0;
    }
}



#define SCREEN_WIDTH 1335
#define ASSETS "" 
void LoadWaterData(void * water_a, size_t size) {
    char filename[40];
    strcpy(filename, ASSETS);
    FILE *f = android_fopen(strcat(filename, "water.data"), "r");
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

    // Write text based file. 
    {
        printf("writing plantdata to sd card using fopen 2\n");
        // @Concerns: Platform layer has to be aware of data for this to work. 
        // @Todo: It would be good to store date, time, and some kind of unique user id.. 
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
			struct Plant * item = &plant_a->items[i];
			if (item.x == NULL || plant_a->items[i].y == NULL || plant_a->items[i].color.r == NULL) {
				continue;
			}
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
        FILE *f = fopen("/sdcard/data_v1.txt", "w");
        if (f) {
            fwrite(temp->data, strlen(temp->data)-1, 1, f);
            fclose(f);
            printf("successfully wrote\n");
        }
        else {
            printf("write failed\n");
        }
        temp->occupied = 0;
    }
}

// :read plants.data
void LoadPlantData(Plant_a * plant_a, size_t size, Temporary_Storage * temp ) {
    FILE *f = fopen("/sdcard/data_v1.txt", "r");
    assert(f);
    // Read first byte of file to see what version it is. 
    int version = 1;
    if (version == 0) {
        // Version 0
        printf("V0 Data - readng sizeof plant_a %zu\n", size );
        if (f != NULL) {
            printf("successfully read \n");
            long int f_len;
            fseek(f, 0, SEEK_END);
            f_len = ftell(f);
            assert(f_len <= size);
            fseek(f, 0, SEEK_SET);
            fread(plant_a, f_len, 1, f);
            fclose(f);
        }
        else {
            printf("Couldn't read data\n");
        }
    }
    if (version == 1) {
		printf("successfully read data \n" );
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
			char * str = strtok_r(row, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].x = atoi(str);
			str = strtok_r(NULL, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].y = atoi(str);
			str = strtok_r(NULL, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].color.r = atoi(str);
			str = strtok_r(NULL, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].color.g = atoi(str);
			str = strtok_r(NULL, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].color.b = atoi(str);
			str = strtok_r(NULL, " ", &end_token);
			if (!str) goto restart;
			plant_a->items[plant_a->count].color.a = atoi(str);
			++plant_a->count;
			restart:
			row = strtok_r (NULL,"\n", &end_str);
	    }
		printf("done reading\n");
		printf("count %d\n", plant_a->count);
	}

}
/*
 *
D/VIBRANT (10128): row 0 0 0 121 241 255
D/VIBRANT (10128): row 0 -1 0 121 221 255
D/VIBRANT (10128): row 3 -2 0 121 241 255
D/VIBRANT (10128): row 3 -2 0 121 241 255
D/VIBRANT (10128): row -3 -1 0 121 241 2555-3 -2 0 121 241 255
D/VIBRANT (10128): row -3 -3 0 121 241 255
D/VIBRANT (10128): row -4 -3 0 121 241 255
D/VIBRANT (10128): row -5  3 0 121 241 255
D/VIBRANT (10128): row 4 -1 0 121 241 255
D/VIBRANT (10128): row 5 0 0 121 241 255
D/VIBRANT (10128): row 7 1 0 121  41 255
D/VIBRANT (10128): row 7 1 0 121 241 255
D/VIBRANT (10128): row 5 1 0 121 241 255
D/VIBRANT (10128): row 2 6 230 41 55 255
D/VIBRANT (10128): row 1   230 41 55 255
 */
void WriteWaterData(void * plant_a, size_t size) {
    // :write plants.data
    char filename[40];
    strcpy(filename, ASSETS);
    FILE *f = android_fopen(strcat(filename, "water.data"), "w");
    if (f) {
        fwrite(plant_a, size, 1, f);
        fclose(f);
    }
}
#include "vibrant.c"

int main(void) {

    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    Temporary_Storage temp;
    temp.size = 100000;
    temp.data = malloc(100000);
    temp.high_water_mark = 0;
    temp.occupied = 0;

    game(&temp);
}
