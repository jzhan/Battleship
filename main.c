#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Coord {
    short x, y;
};

struct Board {
    char *pattern;
    short height, width;
};

struct Node {
    void (*fire)(struct Board *player_board, struct Board *game_board, struct Coord position);
    struct Node *next;
};

struct LinkedList {
    struct Node *head;
};

void push(struct LinkedList *list, void (*missile)(struct Board *player_board, struct Board *game_board, struct Coord position)) {
    struct Node *new_node = malloc(sizeof(struct Node));
    new_node->next = list->head;
    list->head = new_node;
    list->head->fire = missile;
}

void pop(struct LinkedList *list, struct Board *player_board, struct Board *game_board, struct Coord position) {
    if(list->head == NULL) return;

    list->head->fire(player_board, game_board, position);

    struct Node *temp = list->head;
    list->head = list->head->next;

    free(temp);
}

void singleMissile(struct Board *player_board, struct Board *game_board, struct Coord position) {
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

                break;
        }
}

void vLineMissile(struct Board *player_board, struct Board *game_board, struct Coord position) {
    //printf("V-Line Missile\n");

    for(short y = 0; y < player_board->height; ++y) {
        singleMissile(player_board, game_board, (struct Coord) {position.x, y});
    }
}

void hLineMissile(struct Board *player_board, struct Board *game_board, struct Coord position) {
    //printf("H-Line Missile\n");

    for(short x = 0; x < player_board->height; ++x) {
        singleMissile(player_board, game_board, (struct Coord) {x, position.y});
    }
}

void splashMissile(struct Board *player_board, struct Board *game_board, struct Coord position) {
    //printf("Splash Missile\n");

    singleMissile(player_board, game_board, (struct Coord) {position.x, position.y});

    if(position.x - 1 >= 0) {
        singleMissile(player_board, game_board, (struct Coord) {position.x - 1, position.y});
    }

    if(position.x + 1 < player_board->width) {
        singleMissile(player_board, game_board, (struct Coord) {position.x + 1, position.y});
    }

    if(position.y - 1 >= 0) {
        singleMissile(player_board, game_board, (struct Coord) {position.x, position.y - 1});
    }

    if(position.y + 1 < player_board->height) {
        singleMissile(player_board, game_board, (struct Coord) {position.x, position.y + 1});
    }
}

void display(struct Board board) {
    for(int i = 0; i < board.height; ++i) {
        for(int j = 0; j < board.width; ++j) {
            printf("%c", board.pattern[board.width * i + j]);
        }

        printf("\n");
    }
}

int main() {
    struct Board game_board = {
        .width = 10,
        .height = 10
    };

    void (*missile[4])(struct Board *player_board, struct Board *game_board, struct Coord position) = {
       singleMissile, vLineMissile, hLineMissile, splashMissile 
    };

    game_board.pattern = malloc(sizeof(char) * game_board.width * game_board.height);
    
    for(int i = 0, n = game_board.width * game_board.height; i < n; ++i) {
        game_board.pattern[i] = '#';
    }

    struct Board player_board = {
        .pattern = "H######H##"
                   "H######H##"
                   "####H#####"
                   "H#####HHHH"
                   "##########"
                   "H#####HHH#"
                   "H#########"
                   "##H#######"
                   "##########"
                   "##H##HHH##",
        .width = 10,
        .height = 10
    };

    struct LinkedList list = {.head = NULL};

    //push(&list, splashMissile);

    struct Coord position;
    int missile_code;

    while(1) {
        display(game_board);
        printf("\nMissile available:\n");
        printf("1. Single Missile\n");
        printf("2. V-Line Missile\n");
        printf("3. H-Line Missile\n");
        printf("4. Splash Missile\n");

        printf("Input positon (x, y, missile code): ");
        scanf("%d", &position.x);
        scanf("%d", &position.y);
        scanf("%d", &missile_code);

        missile[missile_code - 1](&player_board, &game_board, position);

        printf("====================\n");

        //pop(&list, &player_board, &game_board, position);
    }

    return 0;
}