#include <stdio.h>
#include <memory.h>
#include <miracl/miracl.h>
#include <stdlib.h>
#include <string.h>

#define PERSON  4
#define WALL    5
#define BOX     6
#define DEST    7
#define FREE    0

#define NORTH   7
#define SOUTH   8
#define WEST    9
#define EAST    10


#define QBITS 160
#define PBITS 1024

typedef struct {
    int x;
    int y;
} Point;

Point cp;
char *room;
Point *dest;
int destIp = 0;
int steps = 0;
int currentLevel;
int WIDTH, HEIGHT;
int Status = 0;


int dsasetup(){
    FILE *fp;
    big p,q,h,g,n,s,t;
    long seed;
    miracl *mip=mirsys(100,0);
    p=mirvar(0);
    q=mirvar(0);
    h=mirvar(0);
    g=mirvar(0);
    n=mirvar(0);
    s=mirvar(0);
    t=mirvar(0);

    printf("(debug) Enter 9 digit random number seed  = ");
    scanf("%ld",&seed);
    getchar();
    irand(seed);


    forever {
        bigbits(QBITS,q);
        nxprime(q,q);
        if (logb2(q)>QBITS) continue;
        break;
    }
    
    expb2(PBITS,t);
    decr(t,1,t);
    premult(q,2,n);
    divide(t,n,t);
    expb2(PBITS-1,s);
    decr(s,1,s);
    divide(s,n,s);
    forever {
        bigrand(t,p);
        if (mr_compare(p,s)<=0) continue;
        premult(p,2,p);
        multiply(p,q,p);
        incr(p,1,p);
        copy(p,n);
        if (isprime(p)) break;
    } 
    
    do {
        decr(p,1,t);
        bigrand(t,h);
        divide(t,q,t);
        powmod(h,t,p,g);
    } while (size(g)==1);


    fp=fopen("common.dss","wt");
    fprintf(fp,"%d\n",PBITS);
    mip->IOBASE=16;
    cotnum(p,fp);
    cotnum(q,fp);
    cotnum(g,fp);
    fclose(fp);
    return 0;
}


int dsagen(){
    FILE *fp;
    big p,q,g,x,y;
    long seed;
    int bits;
    miracl *mip;
    
    fp=fopen("common.dss","rt");
    if (fp==NULL){
        printf("file common.dss does not exist\n");
        return 0;
    }
    fscanf(fp,"%d\n",&bits);

    mip=mirsys(bits/4,16);
    p=mirvar(0);
    q=mirvar(0);
    g=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);

    innum(p,fp);
    innum(q,fp);
    innum(g,fp);
    fclose(fp);


    printf("(debug) Enter 9 digit random number seed  = ");
    scanf("%ld",&seed);
    getchar();
    irand(seed);

    powmod(g,q,p,y);
    if (size(y)!=1) {
        printf("Problem - generator g is not of order q\n");
        return 0;
    }


    bigrand(q,x);
    powmod(g,x,p,y);
    
    fp=fopen("public.dss","wt");
    otnum(y,fp);
    fclose(fp);
    fp=fopen("private.dss","wt");
    otnum(x,fp);
    
    fclose(fp);
    mirexit();
    return 0;
}

void hashing(char* msg, int msg_len, big hash)
{ 
    char h[20];
    int i,ch;
    sha sh;
    shs_init(&sh);
    for(i = 0;i<msg_len;i++){
        shs_process(&sh,msg[i]);
    }
    shs_hash(&sh,h);
    bytes_to_big(20,h,hash);
}

int dsasign(char* msg, int msg_len, int print_flag, big prev_k){
    FILE *fp;
    char ifname[50],ofname[50];
    big p,q,g,x,r,s,k,hash;
    long seed;
    int bits;
    miracl *mip;

    
    fp=fopen("common.dss","rt");
    if (fp==NULL) {
        printf("file common.dss does not exist\n");
        return 0;
    }

    fscanf(fp,"%d\n",&bits);
    mip=mirsys(bits/4,16);
    p=mirvar(0);
    q=mirvar(0);
    g=mirvar(0);
    x=mirvar(0);
    r=mirvar(0);
    s=mirvar(0);
    k=mirvar(0);
    hash=mirvar(0);

    innum(p,fp);
    innum(q,fp);
    innum(g,fp);
    fclose(fp);

    if(print_flag){
        printf("(debug) Enter 9 digit random number seed  = ");
        scanf("%ld",&seed);
        getchar();
        irand(seed);
        bigrand(q,k);
    }else{
        k = prev_k;
    }   
    
    
    powmod(g,k,p,r);
    divide(r,q,q);


    fp=fopen("private.dss","rt");
    if (fp==NULL) {
        printf("file private.dss does not exist\n");
        return 0;
    }

    innum(x,fp);
    fclose(fp);
    hashing(msg, msg_len, hash);

    FILE *tmp_fd = fopen("/dev/null","w");
    char* tmp_buf = malloc(100);
    setbuf(tmp_fd, tmp_buf);


    xgcd(k,q,k,k,k);
    mad(x,r,hash,q,q,s);
    mad(s,k,k,q,q,s);
    fp=fopen("signed.out","wt");
    otnum(r,fp);
    
    otnum(r,tmp_fd);
    if(print_flag){
        printf("\nD3CTF{%.16s", tmp_buf);
    }
    fflush(tmp_fd);
    otnum(s,fp);
    otnum(s,tmp_fd);
    if(print_flag)
        printf("%.16s}\n", tmp_buf);
    otnum(k,fp);
    
    free(tmp_buf);
    fclose(tmp_fd);
    fclose(fp);
    xgcd(k,q,k,k,k);
    mirexit();
    return k;
}

int dsaverify(char* msg, int msg_len){
    FILE *fp;
    char ifname[50],ofname[50];
    big p,q,g,y,v,u1,u2,r,s,hash;
    int bits;
    miracl *mip;
    
    fp=fopen("common.dss","rt");
    if (fp==NULL) {
        printf("file common.dss does not exist\n");
        return 0;
    }

    fscanf(fp,"%d\n",&bits);
    mip=mirsys(bits/4,16);
    p=mirvar(0);
    q=mirvar(0);
    g=mirvar(0);
    y=mirvar(0);
    v=mirvar(0);
    u1=mirvar(0);
    u2=mirvar(0);
    s=mirvar(0);
    r=mirvar(0);
    hash=mirvar(0);

    innum(p,fp);
    innum(q,fp);
    innum(g,fp);
    fclose(fp);


    fp=fopen("public.dss","rt");
    if (fp==NULL) {
        printf("file public.dss does not exist\n");
        return 0;
    }
    innum(y,fp);
    fclose(fp);
    

    hashing(msg, msg_len, hash);

    fp=fopen("signed.out","rt");
    if (fp==NULL) { 
        printf("signature file %s does not exist\n","signed.out");
        return 0;
    }
    innum(r,fp);
    innum(s,fp);
    fclose(fp);
    if (mr_compare(r,q)>=0 || mr_compare(s,q)>=0) {
        return 0;
    }
    xgcd(s,q,s,s,s);
    mad(hash,s,s,q,q,u1);
    mad(r,s,s,q,q,u2);
    powmod2(g,u1,y,u2,p,v);
    divide(v,q,q);
    if (mr_compare(v,r)==0){
        // printf("Signature is verified\n");
        return 1;
    }
    // else printf("Signature is NOT verified\n");
    return 0;
}

int getCoord(int x, int y) {
    return (y - 1) * WIDTH + x - 1;
}

char getch() {  //无回显getch
    char c;
    system("stty -echo");
    system("stty -icanon");
    c = getchar();
    system("stty icanon");
    system("stty echo");
    return c;
}

void Move(int Direction) {
    switch (Direction) {
        case NORTH:
            if (room[getCoord(cp.x, cp.y - 1)] == FREE) {
                //往上走，没有墙没有box
                room[getCoord(cp.x, cp.y - 1)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.y--;
                steps++;
            } else if (room[getCoord(cp.x, cp.y - 1)] == BOX && room[getCoord(cp.x, cp.y - 2)] == FREE) {
                //上面有box且box上面没有墙

                room[getCoord(cp.x, cp.y - 2)] = BOX;
                room[getCoord(cp.x, cp.y - 1)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.y--;
                steps++;
            }
            break;
        case SOUTH:
            if (room[getCoord(cp.x, cp.y + 1)] == FREE) {

                room[getCoord(cp.x, cp.y + 1)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.y++;
                steps++;
            } else if (room[getCoord(cp.x, cp.y + 1)] == BOX && room[getCoord(cp.x, cp.y + 2)] == FREE) {

                room[getCoord(cp.x, cp.y + 2)] = BOX;
                room[getCoord(cp.x, cp.y + 1)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.y++;
                steps++;
            }
            break;
        case WEST:
            if (room[getCoord(cp.x - 1, cp.y)] == FREE) {
                room[getCoord(cp.x - 1, cp.y)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.x--;
                steps++;
            } else if (room[getCoord(cp.x - 1, cp.y)] == BOX && room[getCoord(cp.x - 2, cp.y)] == FREE) {

                room[getCoord(cp.x - 2, cp.y)] = BOX;
                room[getCoord(cp.x - 1, cp.y)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.x--;
                steps++;
            }
            break;
        case EAST:
            if (room[getCoord(cp.x + 1, cp.y)] == FREE) {

                room[getCoord(cp.x + 1, cp.y)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.x++;
                steps++;
            } else if (room[getCoord(cp.x + 1, cp.y)] == BOX && room[getCoord(cp.x + 2, cp.y)] == FREE) {
                room[getCoord(cp.x + 2, cp.y)] = BOX;
                room[getCoord(cp.x + 1, cp.y)] = PERSON;
                room[getCoord(cp.x, cp.y)] = FREE;
                cp.x++;
                steps++;
            }
            break;
    }

    display();
}

void keyLoop() {
    while (1) {
        switch (getch()) {
            case 0x1b:
                switch (getch()) {
                    case 0x5b:
                        switch (getch()) {//响应方向键需要检测三次输入
                            case 0x41://north
                                Move(NORTH);
                                break;
                            case 0x42://south
                                Move(SOUTH);
                                break;
                            case 0x43://east
                                Move(EAST);
                                break;
                            case 0x44://west
                                Move(WEST);
                                break;
                            default:
                                break;
                        }
                        if (isWin() == 1) {
                            printf("\n\nYou win, Congratulations~\n");
                            saveScoreboard();
                            return;
                        }
                        break;
                    case 0x1b:
                        return;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

int isDest(int x, int y) {
    int i = 0;
    do {
        if (dest[i].x == x && dest[i].y == y) {
            return 1;
        }
        i++;
    } while (dest[i].x != 0 && dest[i].y != 0);
    return 0;
}

void display() {
    system("clear");
    printf("Level%d, Current step(s): %d\n\n", currentLevel + 1, steps);
    for (int y = 1; y < HEIGHT + 1; y++) {
        for (int x = 1; x < WIDTH + 1; x++) {
            if (isDest(x, y) == 1 && room[getCoord(x, y)] == FREE) {
                printf(".");
            } else {
                switch (room[getCoord(x, y)]) {
                    case BOX:
                        printf("B");
                        break;
                    case FREE:
                        printf(" ");
                        break;
                    case PERSON:
                        cp.x = x;
                        cp.y = y;
                        printf("O");
                        break;
                    case WALL:
                        printf("@");
                        break;
                    default:
                        break;
                }
            }
        }
        printf("\n");
    }
    printf("\n(Press ESC twice to go back)\n");
}

int isWin() {
    int i = 0;
    do {
        if (room[getCoord(dest[i].x, dest[i].y)] != BOX) {
            //目标位置有没有放置好的
            return 0;
        }
        i++;
    } while (dest[i].x != 0 && dest[i].y != 0);
    return 1;
}

//mapdata:
//width height room1 room2 room3 room4 room5 room6 ......
void loadMaps() {
    char map_path[4][20] = {"./level1.map", "./level2.map", "./level3.map", "./level4.map"};

    FILE *file = fopen(map_path[currentLevel], "rb+");
    if (file == NULL) {
        printf("I'm sorry, but where is my mapdata \"%s\" ?\r\n", map_path[currentLevel]);
        return;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 2) {
        printf("Invalid mapdata. Size: %d\r\n", size);
        return;
    }

    char *mapdata = malloc(size);
    memset(mapdata, 0, size);
    fread(mapdata, size, 1, file);
    fclose(file);

    memcpy(&WIDTH, mapdata, 4);
    memcpy(&HEIGHT, mapdata + 4, 4);

    if (WIDTH * HEIGHT != (size - 2 * 4)) {
        printf("Invalid mapdata.\r\n");
        return;
    }

    room = malloc(WIDTH * HEIGHT);
    dest = malloc(WIDTH * HEIGHT * sizeof(Point));
    destIp = 0;
    memset(room, 0, WIDTH * HEIGHT);
    memset(dest, 0, WIDTH * HEIGHT * sizeof(Point));

    memcpy(room, mapdata + 8, WIDTH * HEIGHT);

    for (int y = 1; y < HEIGHT + 1; y++) {
        for (int x = 1; x < WIDTH + 1; x++) {
            if (room[getCoord(x, y)] == DEST) {
                room[getCoord(x, y)] = FREE;
                dest[destIp].x = x;
                dest[destIp].y = y;
                destIp++;
            }

        }
    }
    dest[destIp].x = dest[destIp].y = 0;

    display();
}

void saveScoreboard() {
    FILE *fd;
    fd = fopen("./score.data", "rb+");
    if (fd == NULL) {
        printf("I'm sorry, but where is my scoreBoard data file (score.data) ?\n");
        return;
    }

    char score[16] = {0};
    fread(score, 16, 1, fd);
    fclose(fd);

    fd = fopen("./score.data", "wb+");
    if (((int *) score)[currentLevel] > steps ||
        !((int *) score)[currentLevel]) {
        ((int *) score)[currentLevel] = steps;
        printf("New record for level%d: %d steps.\n\n", currentLevel + 1, steps);
    }

    fwrite(&score, 16, 1, fd);
    fclose(fd);
}

void printScoreboard() {
    FILE *fd;
    fd = fopen("./score.data", "rb+");
    if (fd == NULL) {
        printf("I'm sorry, but where is my scoreBoard data file (score.data) ?\n\n");
        return;
    }
    char score[16] = {0};
    fread(score, 16, 1, fd);
    fclose(fd);
    printf("-----Score Board-----\n");
    for (int i = 0; i < 4; i++) {
        printf("level%d: %d steps\n", i + 1, ((int *) score)[i]);
    }

    printf("\nPress 'c' to clear the score board or any other keys to continue.\n");
    getch();
    if (getch() == 'c') {
        char score[16] = {0};
        fd = fopen("./score.data", "wb+");
        fwrite(&score, 16, 1, fd);
        fclose(fd);
        printf("Cleared.\n");
    }
}


void run_game(char* username){
    int key;
    system("clear");
    printf("Hi %s, welcome to the puuuushBox game ~\n", username);
    if(!strcmp(username, "flag")){
        char r[30] = {0};
        char s[30] = {0};
        FILE *fl = fopen("signed.out", "rt");
        fscanf(fl, "%s", r);
        if(r[strlen(r)-1]=='\n') r[strlen(r)-1] = 0;
        if(s[strlen(s)-1]=='\n') s[strlen(s)-1] = 0;
        fscanf(fl, "%s", s);
        fclose(fl);
        printf("Congratz, your flag is D3CTF{%.16s%.16s}\n", r, s);
    }

    while (1) {
        printf("\nPlease choose a mode:\n0.Score board.\n1.level1.\n2.level2\n3.level3.\n4.level4.\n\n");
        if (scanf("%d", &key) != 0) {
            if (key == 0) {
                system("clear");
                printScoreboard();
            } else if (key > 0 && key < 5) {
                steps = 0;
                currentLevel = key - 1;
                loadMaps();
                keyLoop();
            } else {
                printf("Hello my friend, what did you say?\n\n");
            }
        } else {
            printf("Hello my friend, What did you say?\n\n");
            char clearStdin[100];
            scanf("%s", clearStdin);
        }

    }
}

void clear_crime_scene(){
    system("rm -f private.dss");
}

void gen_signed_flag(){
    dsasetup();
    dsagen();
    char flag[] = "flag";
    char msg[] = "d3ctf";
    big k = dsasign(flag, strlen(flag),1,NULL); // result will only be printed, later it'll be overwritten
    dsasign(msg, strlen(msg),0, k); // this stays
    clear_crime_scene();
}


int main() {
    //char flag[] = "d3ctf";
    //gen_signed_flag(flag);
    //return 0;
    char username[25] = {0};
    printf("Please enter your name first: ");
    scanf("%24s", username);
    if (!strcmp(username, "debug")){
        printf("Great! You find the backdoor used for generating the flag.\n");
        gen_signed_flag();
        return 0;
    }
    if (dsaverify(username, strlen(username))){
        run_game(username);
    }
    else
        printf("License check failed.\n");
    return 0;
}
