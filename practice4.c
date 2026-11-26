#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_SIZE 4
#define MAX_LIST 20
#define LISTNO_OFFSET_GRADE 1

typedef struct {
    char name[50];
    int objectType;
    int type;
    int credit;
    int energy;
    double grade;
} smmObject_t;

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    char name[30];
    int position;
    int credit;
    int energy;
    int graduated;
} smmPlayer_t;

static Node *database[MAX_LIST] = {NULL};

void *allocateMemory(size_t size) {
    void *p = malloc(size);

    if (p == NULL) {
        printf("Memory allocation error\n");
        exit(1);
    }

    return p;
}

void *smmObj_genObject(const char *name,
                       int objectType, int type,
                       int credit, int energy,
                       double grade) {
    smmObject_t *obj;

    obj = allocateMemory(sizeof(smmObject_t));

    strcpy(obj->name, name);
    obj->objectType = objectType;
    obj->type = type;
    obj->credit = credit;
    obj->energy = energy;
    obj->grade = grade;

    return (void *)obj;
}

void smmdb_addTail(int listNo, void *data) {
    Node *newNode = allocateMemory(sizeof(Node));

    newNode->data = data;
    newNode->next = NULL;

    if (database[listNo] == NULL) {
        database[listNo] = newNode;
    }
    else {
        Node *current = database[listNo];

        while (current->next != NULL) {
            current = current->next;
        }

        current->next = newNode;
    }
}

void *smmdb_getData(int listNo, int index) {
    Node *current = database[listNo];

    for (int i = 0; i < index; i++) {
        if (current == NULL)
            return NULL;

        current = current->next;
    }

    if (current == NULL)
        return NULL;

    return current->data;
}

const char *smmObj_getObjectName(void *data) {
    return ((smmObject_t *)data)->name;
}

int smmObj_getObjectType(void *data) {
    return ((smmObject_t *)data)->type;
}

int smmObj_getObjectCredit(void *data) {
    return ((smmObject_t *)data)->credit;
}

int smmObj_getObjectEnergy(void *data) {
    return ((smmObject_t *)data)->energy;
}

double smmObj_getObjectGrade(void *data) {
    return ((smmObject_t *)data)->grade;
}

int checkGrade(int playerNo, const char *name) {
    int listNo = LISTNO_OFFSET_GRADE + playerNo;
    int index = 0;
    void *data;

    while ((data = smmdb_getData(listNo, index))
           != NULL) {

        if (strcmp(smmObj_getObjectName(data),
                   name) == 0)
            return 1;

        index++;
    }

    return 0;
}

void printGrade(int playerNo) {
    int listNo = LISTNO_OFFSET_GRADE + playerNo;
    int index = 0;
    void *data;

    printf("\n===== Grade Report =====\n");

    while ((data = smmdb_getData(listNo, index))
           != NULL) {

        printf("%s : %.1f (%d credits)\n",
               smmObj_getObjectName(data),
               smmObj_getObjectGrade(data),
               smmObj_getObjectCredit(data));

        index++;
    }
}

void stepForward(smmPlayer_t *player, int dice) {
    for (int i = 0; i < dice; i++) {
        player->position++;

        if (player->position >= BOARD_SIZE) {
            player->position = 0;
            player->energy += 5;
        }
    }
}

void actionNode(smmPlayer_t *player, int playerNo) {
    void *data = smmdb_getData(0, player->position);

    int type = smmObj_getObjectType(data);
    int credit = smmObj_getObjectCredit(data);
    int energy = smmObj_getObjectEnergy(data);

    const char *name = smmObj_getObjectName(data);

    if (type == 1) {
        if (checkGrade(playerNo, name)) {
            printf("Already completed: %s\n", name);
        }
        else if (player->energy >= -energy) {
            player->credit += credit;
            player->energy += energy;

            void *grade = smmObj_genObject(
                name, 1, type, credit, 0, 4.0);

            smmdb_addTail(
                LISTNO_OFFSET_GRADE + playerNo,
                grade);

            printf("Course completed: %s\n", name);
        }
    }
    else if (type == 2) {
        player->energy += energy;
    }

    if (player->credit >= 6) {
        player->graduated = 1;
    }
}

void freeDatabase(void) {
    for (int i = 0; i < MAX_LIST; i++) {
        Node *current = database[i];

        while (current != NULL) {
            Node *temp = current;

            current = current->next;

            free(temp->data);
            free(temp);
        }

        database[i] = NULL;
    }
}

int main(void) {
    int playerCount;

    printf("Enter number of players: ");
    scanf("%d", &playerCount);

    if (playerCount < 1 || playerCount > 10) {
        printf("Invalid player count\n");
        return 1;
    }

    smmPlayer_t *player = allocateMemory(
        playerCount * sizeof(smmPlayer_t));

    smmdb_addTail(0, smmObj_genObject(
        "Home", 0, 0, 0, 5, 0));

    smmdb_addTail(0, smmObj_genObject(
        "Programming", 0, 1, 3, -2, 0));

    smmdb_addTail(0, smmObj_genObject(
        "Restaurant", 0, 2, 0, 3, 0));

    smmdb_addTail(0, smmObj_genObject(
        "Mathematics", 0, 1, 3, -2, 0));

    for (int i = 0; i < playerCount; i++) {
        sprintf(player[i].name, "Player%d", i + 1);
        player[i].position = 0;
        player[i].credit = 0;
        player[i].energy = 10;
        player[i].graduated = 0;
    }

    printf("\n===== Sookmyung Marble =====\n");

    for (int turn = 0; turn < 10; turn++) {
        int i = turn % playerCount;
        int dice = rand() % 6 + 1;

        if (player[i].graduated)
            continue;

        stepForward(&player[i], dice);

        void *data = smmdb_getData(
            0, player[i].position);

        printf("\nTurn %d\n", turn + 1);
        printf("Player   : %s\n", player[i].name);
        printf("Dice     : %d\n", dice);
        printf("Position : %s\n",
               smmObj_getObjectName(data));

        actionNode(&player[i], i);

        printf("Credit   : %d\n", player[i].credit);
        printf("Energy   : %d\n", player[i].energy);

        if (player[i].graduated) {
            printf("Winner: %s\n", player[i].name);
            break;
        }
    }

    for (int i = 0; i < playerCount; i++) {
        printf("\nPlayer: %s\n", player[i].name);
        printGrade(i);
    }

    freeDatabase();
    free(player);

    printf("\nMemory released successfully.\n");

    return 0;
}