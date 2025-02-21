#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

#define ROW 17
#define COLUMN 49
#define WALLS 6
#define SHARDS 6
#define HORI_LINE '-'
#define VERT_LINE '|'
#define CORNER '+'
#define PLAYER '0'

//walls
typedef struct {
    int rows;
    int columns;
    int directions;
} Wall;

//goldshards
typedef struct {
    int rows;
    int columns;
    int directions;
    int isthere;
} GoldShard;

// globa; variables
pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
int game = 1;
int player_x;
int player_y;
char map[ROW][COLUMN + 1];
int collected_shards = 0;
Wall walls[WALLS];
GoldShard shards[SHARDS];

// declaring prototypes
int kbhit(void);
void map_print(void);
void init_walls();
void init_shards();
void* handle_input(void* arg);
void* walls_movement(void* arg);
void* goldshards_movement(void* arg);

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void map_print(void)
{
    printf("\033[H\033[2J");
    int i;
    for (i = 0; i <= ROW - 1; i++) //map row 0-16   0-row-1         column 0-48     0-column-1
        puts(map[i]);
}

void init_walls() {
    int init_rows[WALLS] = {2, 4, 6, 10, 12, 14};
    for(int i = 0 ; i < WALLS ; i++){
        walls[i].rows = init_rows[i];
        walls[i].columns = rand() % (COLUMN - 15 - 2) + 1;
        walls[i].directions = (i % 2 == 0) ? 1 : -1;
    }
}

void init_shards() {
    int init_rows[SHARDS] = {1, 3, 5, 11, 13, 15};
    for (int i = 0; i < SHARDS; i++) {
        shards[i].rows = init_rows[i];
        shards[i].columns = rand() % (COLUMN - 2) + 1;
        shards[i].directions = (rand() % 2 == 0) ? 1 : -1;
        shards[i].isthere = 1;
    }
}

void* handle_input(void* arg) {
    while(game == 1){
        if(kbhit()){
            char kb = getchar();
            pthread_mutex_lock(&map_mutex);

            map[player_x][player_y] = ' ';

            if(kb == 'q'  || kb == 'Q'){
                game = 0;
                printf("\033[H\033[2J");
                printf("You exit the game.\n");
            }
            if(kb == 'w' || kb == 'W'){
                if(player_x > 1){ // 0 1
                    player_x--;
                }
            }
            if(kb == 's' || kb == 'S'){
                if(player_x < ROW - 2){
                    player_x++;
                }
            }
            if(kb == 'a' || kb == 'A'){
                if(player_y > 1){
                    player_y--;
                }
            }
            if(kb == 'd' || kb == 'D'){
                if(player_y < COLUMN - 2){
                    player_y++;
                }
            }
            if (map[player_x][player_y] == '$') {
                for (int i = 0; i < SHARDS; i++) {
                    if (shards[i].rows == player_x && shards[i].columns == player_y && shards[i].isthere && shards[i].isthere == 1) {
                        shards[i].isthere = 0;
                        collected_shards++;
                    }
                }
                if (collected_shards >= SHARDS) {
                    printf("\033[H\033[2J");
                    printf("You win the game!!\n");
                    game = 0;
                }
            }
            map[player_x][player_y] = PLAYER;

            pthread_mutex_unlock(&map_mutex);
        }
    }
    pthread_exit(NULL);
}

void* walls_movement(void* arg) {
    while (game == 1) {
        pthread_mutex_lock(&map_mutex);

        for (int i = 0; i < WALLS; i++) {
            for (int j = 0; j < 15; j++) {
                int current_col = ((walls[i].columns - 1 + j) % (COLUMN - 2)) + 1;
                if (walls[i].rows == player_x && current_col == player_y) {
                    printf("\033[H\033[2J");
                    printf("You lose the game!!\n");
                    game = 0;
                }
            }
        }
        for (int i = 0; i < WALLS; i++) {
            for (int j = 0; j < 15; j++) {
                int current_col = ((walls[i].columns - 1 + j) % (COLUMN - 2)) + 1;
                map[walls[i].rows][current_col] = ' ';
            }
        }
        for (int i = 0; i < WALLS; i++) {
            walls[i].columns = ((walls[i].columns - 1 + walls[i].directions + (COLUMN - 2)) % (COLUMN - 2)) + 1;
        }
        for (int i = 0; i < WALLS; i++) {
            for (int j = 0; j < 15; j++) {
                int current_col = ((walls[i].columns - 1 + j) % (COLUMN - 2)) + 1;
                map[walls[i].rows][current_col] = '=';
            }
        }
        pthread_mutex_unlock(&map_mutex);

        usleep(30000);
    }
    pthread_exit(NULL);
}

void* goldshards_movement(void* arg) {
    while (game == 1) {
        pthread_mutex_lock(&map_mutex);

        for (int i = 0; i < SHARDS; i++) {
            if (shards[i].isthere) {
                int current_col = ((shards[i].columns - 1) % (COLUMN - 2)) + 1;
                map[shards[i].rows][current_col] = ' ';
            }
        }
        for (int i = 0; i < SHARDS; i++) {
            if (shards[i].isthere) {
                shards[i].columns = ((shards[i].columns - 1 + shards[i].directions + (COLUMN - 2)) % (COLUMN - 2)) + 1;
                if (shards[i].rows == player_x && shards[i].columns == player_y && shards[i].isthere == 1) {
                    shards[i].isthere = 0;
                    collected_shards++;
                    if (collected_shards >= SHARDS) {
                        printf("\033[H\033[2J");
                        printf("You win the game!!\n");
                        game = 0;
                    }
                }
            }
        }
        for (int i = 0; i < SHARDS; i++) {
            if (shards[i].isthere) {
                int current_col = ((shards[i].columns - 1) % (COLUMN - 2)) + 1;
                map[shards[i].rows][current_col] = '$';
            }
        }
        pthread_mutex_unlock(&map_mutex);
        usleep(700000);
    }
    pthread_exit(NULL);
}

/* main function */
int main(int argc, char *argv[]) {
    pthread_t walls_thread, shards_thread, input_thread;

    srand(time(NULL));
    int i, j;

    /* initialize the map */
    memset(map, 0, sizeof(map));
    for (i = 1; i <= ROW - 2; i++){
        for (j = 1; j <= COLUMN - 2; j++)
        {
            map[i][j] = ' ';
        }
    }
    for (j = 1; j <= COLUMN - 2; j++){
        map[0][j] = HORI_LINE;
        map[ROW - 1][j] = HORI_LINE;
    }
    for (i = 1; i <= ROW - 2; i++){
        map[i][0] = VERT_LINE;
        map[i][COLUMN - 1] = VERT_LINE;
    }
    map[0][0] = CORNER;
    map[0][COLUMN - 1] = CORNER;
    map[ROW - 1][0] = CORNER;
    map[ROW - 1][COLUMN - 1] = CORNER;

    player_x = ROW / 2;
    player_y = COLUMN / 2;
    map[player_x][player_y] = PLAYER;

    init_shards();
    init_walls();
    map_print();

    pthread_create(&input_thread, NULL, handle_input, NULL);
    pthread_create(&walls_thread, NULL, walls_movement, NULL);
    pthread_create(&shards_thread, NULL, goldshards_movement, NULL);

    while (game == 1) {
        pthread_mutex_lock(&map_mutex);

        for (i = 1; i <= ROW - 2; i++) {
            for (j = 1; j <= COLUMN - 2; j++) {
                map[i][j] = ' ';
            }
        }
        for (i = 0; i < WALLS; i++) {
            for (j = 0; j < 15; j++) {
                int current_col = ((walls[i].columns - 1 + j) % (COLUMN - 2)) + 1;
                map[walls[i].rows][current_col] = '=';
            }
        }
        for (i = 0; i < SHARDS; i++) {
            if (shards[i].isthere) {
                int current_col = ((shards[i].columns - 1) % (COLUMN - 2)) + 1;
                map[shards[i].rows][current_col] = '$';
            }
        }
        map[player_x][player_y] = PLAYER;

        pthread_mutex_unlock(&map_mutex);
        map_print();

        usleep(50000);
    }

    pthread_join(input_thread, NULL);
    pthread_join(walls_thread, NULL);
    pthread_join(shards_thread, NULL);

    pthread_mutex_destroy(&map_mutex);

    return 0;
}
