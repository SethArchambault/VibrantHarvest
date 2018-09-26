

#define Role_Max 6
#define Tile_Size 64
#define Water_Max 10000


typedef struct {
    int x;
    int y;
} Coords;


typedef struct {
    Color color;
    char desc[20];
} Role;

typedef struct {
    int count;
    Role items[Role_Max];
} Role_a;

typedef struct {
    Coords coord;
    int x;
    int y;
} Water;


typedef struct {
    Water items[Water_Max];
    int count;
} Water_a;


void print_plant_a(char msg[30],Plant_a *plant_a) {
    printf("\n%s plant_a.count %d \n", msg, plant_a->count);
    for(int i = 0; i < plant_a->count; ++i) {
        printf("    %d x:%2d y:%2d\n", i, plant_a->items[i].x, plant_a->items[i].y);
    }
}

typedef enum {
    TITLE, PLAY
} Mode;
typedef enum {
    STAND, WALK
} Action;

Vector2 v2_pixel_from_v2_coord(Vector2 coord) {
    coord.x *= 64;
    coord.y *= 64;
    return coord;
}

typedef struct {
    int x;
    int y;
    int x_dest;
    int y_dest;
    float x_pixel;
    float y_pixel;
    float x_pixel_dest;
    float y_pixel_dest;
    float distance;
    bool moving_x;
    bool moving_y;
} Player;


int int_from_float(float f) {
    int i;
    int precision = 100;
    if (f > 0) {
        i = (int)(f * precision + .5);
    } else {
        i = (int)(f * precision - .5);
    }
    return i;
}
void game(Temporary_Storage * temp) {
    printf("SETH starting game\n");
    // :width
    // :height
    Vector2 screen = {SCREEN_WIDTH, 768}; //desktop
    //Vector2 screen = {1335, 768}; tablet
    //Vector2 screen = {1366, 768};

    InitWindow(screen.x, screen.y, "Vibrant");
    SetTargetFPS(60);

    char filename[40];
    strcpy(filename, ASSETS);
    Texture2D ground_tex            = LoadTexture(strcat(filename, "ground.png"));
    strcpy(filename, ASSETS);
    Texture2D plant_tex             = LoadTexture(strcat(filename, "plant.png"));
    strcpy(filename, ASSETS);
    Texture2D person_stand_tex      = LoadTexture(strcat(filename, "white_man_standing.png"));
    strcpy(filename, ASSETS);
    Texture2D person_walk_tex       = LoadTexture(strcat(filename, "white_man_walking.png"));
    Vector2 cursor; 

    Mode mode = TITLE; 

    Role_a role_a = {
        0,
        {
            {BLUE,   "Quebequa"}, 
            {ORANGE, "Ashkenazi"}, 
            {RED,    "Cameroon"}, 
            {GREEN,  "Gabon"}, 
            {GRAY,   "Congo"}, 
            {PINK,   "Bengali"}
        }
    };

    int frame_passed = 0;
    int frame = 0;
    while (!WindowShouldClose()) {
        ++frame_passed;
        // :character selection
        if (TITLE == mode) {
            if (actionPressed()) {
                mode = PLAY;
            }
            if (upPressed()) {
                if (role_a.count > 0) role_a.count--;
            }
            if (downPressed()) {
                if (role_a.count < Role_Max -1) role_a.count++;
            }
            BeginDrawing();
                ClearBackground(WHITE);
                cursor.x =  200;
                cursor.y = screen.y/2 - 120;
                DrawText("VIBRANT",
                        cursor.x, // xpos
                        cursor.y, // ypos
                        80, // fontsize
                        BLACK //textColor
                        );
                cursor.y += 70;
                DrawText("HARVEST",
                        cursor.x, // xpos
                        cursor.y, // ypos
                        80, // fontsize
                        BLACK //textColor
                        );
                cursor.x =  screen.x/2 +100;
                cursor.y = 150;
                DrawText("Who Are Your People?",
                        cursor.x, // xpos
                        cursor.y, // ypos
                        40, // fontsize
                        BLACK //textColor
                        );
                cursor.y += 74;
                if (frame_passed > 10) {
                    frame_passed = 0;
                    ++frame;
                    if (frame > 4) frame = 0;
                }

                for (int i = 0; i < Role_Max; ++i) {
                    if (role_a.count == i) {
                        DrawTexturePro(person_walk_tex, (Rectangle) { 32 * frame, 0, 32, 32 }, (Rectangle) { cursor.x-5, cursor.y-5, 64+10, 64+10}, (Vector2){ 0, 0 }, 0, Fade(role_a.items[role_a.count].color, 1.0));
                        //DrawRectangle(cursor.x-5, cursor.y-5, 64+10, 64+10, role_a.items[i].color);
                        DrawText(role_a.items[i].desc,
                            cursor.x+74, // xpos
                            cursor.y+10, // ypos
                            50, // fontsize
                            BLACK //textColor
                            );
                    } else {
                        //DrawRectangle(cursor.x, cursor.y, 64, 64, role_a.items[i].color);
                        DrawTexturePro(person_stand_tex, (Rectangle) { 32 * frame, 0, 32, 32 }, (Rectangle) { cursor.x, cursor.y, 64, 64}, (Vector2){ 0, 0 }, 0, Fade(role_a.items[i].color, 1.0));
                        DrawText(role_a.items[i].desc,
                            cursor.x+74, // xpos
                            cursor.y+17, // ypos
                            40, // fontsize
                            BLACK //textColor
                            );
                    }

                    cursor.y += 74;
                }
            EndDrawing();
        }
        // :play
        if (PLAY == mode) {
            static int init = 0;
            static Player player;
            static Plant_a *plant_a;
            static Water_a *water_a;
            static Camera2D camera;
            if (init == 0) {
                init                = 1;
                player.x              = 0;
                player.y              = 0;
                player.x_dest              = 0;
                player.y_dest              = 0;
                player.y_pixel = 0;
                player.x_pixel = 0;
                player.distance     = 0;
                player.moving_x     = false;
                player.moving_y     = false;
                player.y_pixel_dest = player.y_pixel;
                player.x_pixel_dest = player.x_pixel;
                camera.rotation     = 0.0f;
                camera.zoom         = 1.5f;
                plant_a = malloc(sizeof(Plant_a));
                water_a = malloc(sizeof(Water_a));
                LoadPlantData(plant_a, sizeof (Plant_a), temp);
                LoadWaterData(water_a, sizeof (Water_a));
            }
            if (!player.moving_y && !player.moving_x && actionPressed()) {
                plant_a->items[plant_a->count].x = player.x;
                plant_a->items[plant_a->count].y = player.y;
                plant_a->items[plant_a->count].color = role_a.items[role_a.count].color;
                ++plant_a->count;
                assert(plant_a->count < Plant_Max);
                WritePlantData(plant_a, sizeof (Plant_a), temp);
            }
            if (!player.moving_y && !player.moving_x && waterPressed()) {
                water_a->items[water_a->count].x = player.x;
                water_a->items[water_a->count].y = player.y;
                ++water_a->count;
                assert(water_a->count < Water_Max);
                WriteWaterData(water_a, sizeof (Water_a));
            }

            if (   up()   && !player.moving_y) {
                player.y_pixel_dest -= Tile_Size;
                player.y_dest   -= 1;
                player.moving_y = 1;
            }
            if ( down()   && !player.moving_y) {
                player.y_pixel_dest += Tile_Size;
                player.y_dest += 1;
                player.moving_y = 1;
            }
            {
                int water_count = 0;
                for (int i = 0; i <  water_a->count; ++i) {
                    if (water_a->items[i].x == player.x_dest && 
                        water_a->items[i].y == player.y_dest ){
                        ++water_count;
                    }
                }
                if (water_count > 1) {
                    player.y_dest = player.y;
                    player.y_pixel_dest = player.y_pixel;
                }
            }
            if (right()   && !player.moving_x) {
                player.x_pixel_dest += Tile_Size;
                player.x_dest   += 1;
                player.moving_x = 1;
            }
            if ( left()   && !player.moving_x) {
                player.x_pixel_dest -= Tile_Size;
                player.x_dest   -= 1;
                player.moving_x = 1;
            }
            {
            int water_count = 0;
                for (int i = 0; i <  water_a->count; ++i) {
                    if (water_a->items[i].x == player.x_dest && 
                        water_a->items[i].y == player.y_dest ){
                        water_count++;
                    }
                }
                if (water_count > 1) {
                    player.x_dest = player.x;
                    player.x_pixel_dest = player.x_pixel;
                }
            }
            static float speed = 4.0f; 
            if (player.x_pixel < player.x_pixel_dest)       player.x_pixel += speed;
            else if (player.x_pixel > player.x_pixel_dest)  player.x_pixel -= speed;
            else {
                player.moving_x = 0;
                player.x = player.x_dest;
                player.x_pixel = player.x * Tile_Size;
                player.x_pixel_dest = player.x_pixel;
            }
            if (player.y_pixel < player.y_pixel_dest)       player.y_pixel += speed;
            else if (player.y_pixel > player.y_pixel_dest)  player.y_pixel -= speed;
            else {
                player.moving_y = 0;
                player.y = player.y_dest;
                player.y_pixel = player.y * Tile_Size;
                player.y_pixel_dest = player.y_pixel;
            }


            camera.offset.x     = screen.x/2 - 32 + -(player.x_pixel * camera.zoom);
            camera.offset.y     = screen.y/2 - 32 + -(player.y_pixel * camera.zoom);
            // :draw
            BeginDrawing();
                ClearBackground(WHITE);
                BeginMode2D(camera); // All that happens in here will move with the camera
                    // :ground
                    for (int y = -10; y < 50; ++y) {
                        for (int x = -10; x < 50; ++x) {
                            if (x > player.x-10) {
                                DrawTextureEx(ground_tex, (Vector2) { x * 64, y * 64 }, 0, 4, WHITE); 
                            }
                        } // x
                    } // y
                    //   :draw water
                    for (int i = 0; i < water_a->count; ++i) {
                        DrawRectangle(water_a->items[i].x * 64, water_a->items[i].y * 64, 64, 64, Fade(BLUE, 0.75f));
                    }
                    //   :draw plant 
                    for (int i = 0; i < plant_a->count; ++i) {
                        DrawTextureEx(plant_tex, (Vector2) { plant_a->items[i].x * 64, plant_a->items[i].y * Tile_Size - Tile_Size / 4  }, 0, 4, Fade(plant_a->items[i].color, 0.5f));
                    }
                    /* different ways to animate.. 
                     * player has a dest
                     * If player.src != player.dest then do an elastic move
                     */
                    static Action player_action = STAND;

                    static float breathing_freq = 10;
                    if ((player.moving_y || player.moving_x) && frame_passed > 5) { 
                        frame_passed = 0;
                        ++frame;
                        breathing_freq *= .85;
                        if (breathing_freq < 1) breathing_freq = 1;
                        if (frame > 4) frame = 0;
                        player_action = WALK;
                    } else if (frame_passed > breathing_freq) { 
                        frame_passed = 0;
                        ++frame;
                        breathing_freq *= 1.10;
                        if (breathing_freq > 13) breathing_freq = 13;
                        if (frame > 4) frame = 0;
                        player_action = STAND;
                    }
                    // :draw player
                    static Rectangle player_rect; 
                    player_rect.height = Tile_Size;
                    player_rect.width = Tile_Size;
                    player_rect.x = player.x_pixel;
                    player_rect.y = player.y_pixel - Tile_Size / 4;
                    if (player_action == STAND) {
                        DrawTexturePro(person_stand_tex, (Rectangle) { 32 * frame, 0, 32, 32 }, player_rect, (Vector2){ 0, 0 }, 0, Fade(role_a.items[role_a.count].color, 1.0));
                    } else {
                        DrawTexturePro(person_walk_tex, (Rectangle) { 32 * frame, 0, 32, 32 }, player_rect, (Vector2){ 0, 0 }, 0, Fade(role_a.items[role_a.count].color, 1.0));
                    }
                EndMode2D();
                DrawTextureEx(plant_tex, (Vector2) { screen.x /2 - 32, 32}, 0, 4, Fade(role_a.items[role_a.count].color, 1.0f));
               // DrawFPS(10, 10);
            EndDrawing();
        }
    }//while
}// main
