#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include<unistd.h>
#define MEM_SIZE 1000

extern int errno;

void print(char table[]) // prints the table
{
    system("clear");
    printf("%c | %c | %c\n", table[0], table[1], table[2]);
    printf("%c | %c | %c\n", table[3], table[4], table[5]);
    printf("%c | %c | %c\n", table[6], table[7], table[8]);
}

int check_win(char table[]) // checks who won and returns 0 if the game has not ended, -1 if there is a draw or letter of the player who won
{
    char player[2]="xo";

    for(int i=0; i<=6; i=i+3)
    {
        if(table[i]==table[i+2] && table[i]==table[i+1] && table[i]!=' ') return table[i];
    }
    for(int i=0; i<=2; i++)
    {
        if(table[i]==table[i+3] && table[i]==table[i+6] && table[i]!=' ') return table[i];
    }
    for(int i=0; i<=2; i=i+2)
    {
        if(table[i]==table[4-i] && table[i]==table[8-i] && table[i]!=' ') return table[i];
    }
    for(int i=0; i<9; i++)
    {
        if(table[i]==' ') return 0;
    }
    return -1;
}

int move_get() // gets a move (place to insert letter) from the player
{
    int place;
    printf("\nEnter place <1-9> to insert a sign:\n");
    scanf(" %d", &place);
    return place;
}


void move(char *table) // inserts move letter into the table
{
    int place=move_get();
    while((place>9 | place<1) | table[place-1]!=' ')
    {
        printf("Wrong move!\n");
        place=move_get();
    }
    table[place-1]=table[9];
    if(table[9]=='X') table[9]='O';
    else table[9]='X';
}

void win_message(int win_status, int player) // shows winner info message
{
    if(win_status==player)
    {
        printf("You won:)\n");
    }
    else if (win_status=='X' ) printf("Player X won.\n");
    else if(win_status=='O') printf("Player O won.\n");
    else if(win_status==-1)
    {
        printf("Draw.\n");
    }
}

//MAIN//

int main(int argc, char *argv[])
{
    char plansza[]="         X";
    char* plansza2;
    int win_status=0;
    int er;
    char player='X';


    key_t key;
    if (argc == 1)
    {
        key = ftok("/tmp", 'a');
    }
    else if (argc == 2)
    {
        key = ftok(argv[1], 'a');
    }
    else
    {
        fprintf(stderr, "usage: %s key_name\n", argv[0]);
        return 1;
    }


    if(key < 0)
    {
        fprintf(stderr, "Error creating key\n");
        return 2;
    }

    int shmid = shmget(key, MEM_SIZE, 0666 | IPC_CREAT | IPC_EXCL); // creating/getting id of a shared memory segment

    if(shmid < 0)
    {
        fprintf(stderr, "Error getting shared memory\n");
        perror("shmget");
        strerror(er);
        printf("%dER\n", errno);
        if(errno==17 && player=='X')
        {
            player='O';
            printf("player X, changing to O\n");
            shmid = shmget(key, MEM_SIZE, 0666);
        }
        else if(errno==17 && player=='O')
        {
            fprintf(stderr, "Error getting shared memory\n");
            return 3;
        }
        else
        {
            fprintf(stderr, "Error getting shared memory\n");
            return 3;
        }
    }

    void *memseg = shmat(shmid, NULL, 0); // attaching to the shared memory segment


    if(memseg == (void *)-1)
    {
        fprintf(stderr, "Error attaching shared memory segment\n");
        perror("smhat");
        return 4;
    }

    if(player=='X')
    {
        strcpy(memseg, plansza);
    }

    while(win_status==0)
    {
        plansza2=memseg;
        print(plansza2);
        win_status=check_win(plansza2);
        win_message(win_status, player);


        if(win_status!=0)
        {
            break;
        }
        printf("Wait for the other player move.\n");

        while(plansza2[9]!=player)
        {
            sleep(1);
        }
        print(plansza2);
        win_status=check_win(plansza2);
        if(win_status!=0)
        {
            win_message(win_status, player);
            break;
        }

        if(win_status==0) move(plansza2);
        strcpy(memseg, plansza2);
        print(plansza2);
    }


    if(shmdt(memseg) == -1)
    {
        fprintf(stderr, "Error detaching shared memory segment\n");
        return 5;
    }

    if(shmctl (shmid, IPC_RMID, NULL) == -1 && win_status==player)
    {
        fprintf(stderr, "Error removing sharem memory\n");
        return 6;
    }
    return 0;
}
