#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

struct Coord {
    short x, y;
};

struct Board {
    char *pattern;
    short height, width;
};

struct Vehicle {
    char name[100];
    struct Coord position;
    int length;
    int health;
    /* N north E east S south W west */
    char direction;
};

struct Ship {
    struct Vehicle *v;
    int n;
};

char (*destroyed_ship)[100];
int num_of_destroyed_ship;

/* Linked List */
struct Node {
    void (*fire)(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position);
    struct Node *next;
};

struct LinkedList {
    struct Node *head;
};

void push(struct LinkedList *list, void (*missile)(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position)) {
    struct Node *new_node = malloc(sizeof(struct Node));
    new_node->next = list->head;
    list->head = new_node;
    list->head->fire = missile;
}

void pop(struct LinkedList *list, struct Board *player_board, struct Ship *ships, struct Board *game_board, struct Coord position) {
    if(list->head == NULL) return;

    list->head->fire(player_board, game_board, ships, position);
    
    if(num_of_destroyed_ship > 0) {
        for(int i = 0; i < num_of_destroyed_ship; ++i) {
            printf("%s destroyed\n", destroyed_ship[i]);
        }

        num_of_destroyed_ship = 0;
    }

    struct Node *temp = list->head;
    list->head = list->head->next;

    free(temp);
}
/* end Linked List */

/* Missile */
void singleMissile(struct Board *player_board, struct Board *game_board, struct Ship *ships , struct Coord position) {
    if(position.x >= player_board->width || position.x < 0 ||
       position.y >= player_board->height || position.y < 0) return;

    //printf("Single Missile\n");

    if(position.x >= 0 && position.x < player_board->width &&
        position.y >= 0 && position.y < player_board->height)
        switch(player_board->pattern[player_board->width * position.y + position.x]) {
            case '#':
                game_board->pattern[game_board->width * position.y + position.x] = 'X';

                break;
            case 'H':
                game_board->pattern[game_board->width * position.y + position.x] = 'O';
                for(int i = 0; i < ships->n; ++i) {
                    for(int j = 0, x_step = 0, y_step = 0; j < ships->v[i].length; ++j) {
                        if(position.x == ships->v[i].position.x + x_step &&
                            position.y == ships->v[i].position.y + y_step) {
                            ships->v[i].health -= 1;

                            if(ships->v[i].health == 0) {
                                strcpy(destroyed_ship[num_of_destroyed_ship], ships->v[i].name);
                                num_of_destroyed_ship += 1;
                            }
                        }

                        switch(ships->v[i].direction) {
                            case 'N':
                                y_step += 1;
                                break;
                            case 'E':
                                x_step -= 1;
                                break;
                            case 'S':
                                y_step -= 1;
                                break;
                            case 'W':
                                x_step += 1;
                                break;
                        }
                    }
                }
                break;
        }
}

void vLineMissile(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position) {
    //printf("V-Line Missile\n");

    for(short y = 0; y < player_board->height; ++y) {
        singleMissile(player_board, game_board, ships, (struct Coord) {position.x, y});
    }
}

void hLineMissile(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position) {
    //printf("H-Line Missile\n");

    for(short x = 0; x < player_board->height; ++x) {
        singleMissile(player_board, game_board, ships, (struct Coord) {x, position.y});
    }
}

void splashMissile(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position) {
    //printf("Splash Missile\n");

    singleMissile(player_board, game_board, ships, (struct Coord) {position.x, position.y});

    if(position.x - 1 >= 0) {
        singleMissile(player_board, game_board, ships, (struct Coord) {position.x - 1, position.y});
    }

    if(position.x + 1 < player_board->width) {
        singleMissile(player_board, game_board, ships, (struct Coord) {position.x + 1, position.y});
    }

    if(position.y - 1 >= 0) {
        singleMissile(player_board, game_board, ships, (struct Coord) {position.x, position.y - 1});
    }

    if(position.y + 1 < player_board->height) {
        singleMissile(player_board, game_board, ships, (struct Coord) {position.x, position.y + 1});
    }
}

/* end Missile */

void display(struct Board board) {
    for(int i = 0; i < board.height; ++i) {
        if(i == 0)
            printf("\xda");
        else
            printf("\xc3");

        for(int k = 0; k <= board.width; ++k) {
            printf("\xc4\xc4\xc4");

            if(i == 0 && k == board.width)
                printf("\xbf");
            else if(i == 0)
                printf("\xc2");
            else if(k < board.width)
                printf("\xc5");
            else
                printf("\xb4");
        }

        if(i == 0) {
            printf("\n\xb3");

            for(int k = 0; k <= board.width; ++k) {
                printf(" %c \xb3", k? 65 + k - 1: '\xfe');
            }

            printf("\n\xc3");

            for(int k = 0; k <= board.width; ++k) {
                printf("\xc4\xc4\xc4");

                if(k < board.width)
                    printf("\xc5");
                else
                    printf("\xb4");
            }
        }

        printf("\n\xb3 %d \xb3", i);
        for(int j = 0; j < board.width; ++j) {
            switch(board.pattern[board.width * i + j]) {
                case '#':
                    printf(BLUE);
                    break;
                case 'H':
                case 'O':
                    printf(GREEN);
                    break;
                case 'X':
                    printf(RED);
                    break;
            }

            printf(" %c %s\xb3", board.pattern[board.width * i + j], RESET);
        }

        printf("\n");
    }

    printf("\xc0");

    for(int k = 0; k <= board.width; ++k) {
        printf("\xc4\xc4\xc4");

        if(k < board.width)
            printf("\xc1");
        else
            printf("\xd9");
    }

    printf("\n");
}

int generatePlayerBoard(struct Board *board, struct Ship *ships, int n) {
    for(int i = 0; i < ships->n; ++i) {
        for(int j = 0, x_step = 0, y_step = 0; j < ships->v[i].length; ++j) {
            board->pattern[board->width * (ships->v[i].position.y + y_step) + ships->v[i].position.x + x_step] = 'H';

            switch(ships->v[i].direction) {
                case 'N':
                    y_step += 1;
                    break;
                case 'E':
                    x_step -= 1;
                    break;
                case 'S':
                    y_step -= 1;
                    break;
                case 'W':
                    x_step += 1;
                    break;
            }
        }
    }
} 

int main() {
    struct Board game_board = {
        .width = 10,
        .height = 10
    };

    void (*missile[4])(struct Board *player_board, struct Board *game_board, struct Ship *ships, struct Coord position) = {
       singleMissile, vLineMissile, hLineMissile, splashMissile 
    };

    game_board.pattern = malloc(sizeof(char) * game_board.width * game_board.height);
    
    for(int i = 0, n = game_board.width * game_board.height; i < n; ++i) {
        game_board.pattern[i] = '#';
    }

    struct Vehicle v[10] = {
        {
            .name = "A",
            .position = {.x = 0, .y = 0},
            .length = 2,
            .health = 2,
            .direction = 'N'
        },
        {
            .name = "B",
            .position = {.x = 7, .y = 1},
            .length = 2,
            .health = 2,
            .direction = 'S'
        },
        {
            .name = "C",
            .position = {.x = 4, .y = 2},
            .length = 1,
            .health = 1,
            .direction = 'S'
        },
        {
            .name = "D",
            .position = {.x = 0, .y = 3},
            .length = 1,
            .health = 1,
            .direction = 'S'
        },
        {
            .name = "E",
            .position = {.x = 6, .y = 3},
            .length = 4,
            .health = 4,
            .direction = 'W'
        },
        {
            .name = "F",
            .position = {.x = 0, .y = 6},
            .length = 2,
            .health = 2,
            .direction = 'S'
        },
        {
            .name = "G",
            .position = {.x = 8, .y = 5},
            .length = 3,
            .health = 3,
            .direction = 'E'
        },
        {
            .name = "H",
            .position = {.x = 2, .y = 7},
            .length = 1,
            .health = 1,
            .direction = 'S'
        },
        {
            .name = "I",
            .position = {.x = 2, .y = 9},
            .length = 1,
            .health = 1,
            .direction = 'S'
        },
        {
            .name = "J",
            .position = {.x = 5, .y = 9},
            .length = 3,
            .health = 3,
            .direction = 'W'
        }
    };

    struct Ship ships = {
        .v = v,
        .n = 10
    };

    struct Board player_board = {
        // .pattern = "H######H##"
        //            "H######H##"
        //            "####H#####"
        //            "H#####HHHH"
        //            "##########"
        //            "H#####HHH#"
        //            "H#########"
        //            "##H#######"
        //            "##########"
        //            "##H##HHH##",
        .width = 10,
        .height = 10
    };

    player_board.pattern = malloc(sizeof(char) * player_board.width * player_board.height);

    for(int i = 0, n = player_board.width * player_board.height; i < n; ++i) {
        player_board.pattern[i] = '#';
    }

    generatePlayerBoard(&player_board, &ships, 10);

    struct LinkedList list = {.head = NULL};

    int n = game_board.width > game_board.height? game_board.width: game_board.height;
    destroyed_ship = malloc(sizeof(char[100]) * n);
    num_of_destroyed_ship = 0;
    //push(&list, splashMissile);

    struct Coord position;
    int missile_code;
    char pos[3];

    display(game_board);

    while(1) {
        printf("\nMissile available:\n");
        printf("1. Single Missile\n");
        printf("2. V-Line Missile\n");
        printf("3. H-Line Missile\n");
        printf("4. Splash Missile\n");

        printf("Input positon (x, y, missile code): ");
        fgets(pos, 3, stdin);
        scanf("%d", &missile_code);
        while(getchar() != '\n');

        position.x = toupper(pos[0]) - 65;
        position.y = pos[1] - 48;

        missile[missile_code - 1](&player_board, &game_board, &ships, position);

        if(num_of_destroyed_ship > 0) {
            for(int i = 0; i < num_of_destroyed_ship; ++i) {
                printf("%s destroyed\n", destroyed_ship[i]);
            }

            num_of_destroyed_ship = 0;
        }

        int flag = 0;

        for(int i = 0; i < ships.n; ++i) {
            if(ships.v[i].health > 0) {
                flag = 1;
                break;
            }
        }

        display(game_board);

        if(flag == 0) break;

        //pop(&list, &player_board, &game_board, position);
    }

    return 0;
}
