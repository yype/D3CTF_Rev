#include <stdio.h>
#include <memory.h>
#include <miracl/miracl.h>
#include <stdlib.h>
#include <string.h>

void hashing(char* msg, int msg_len, big hash)
{ /* compute hash function */
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

int main(){
    FILE *fp;
    big p,q,g,x,y,r,s,k,hash,tmp0,tmp1;
    char msg[] = "d3ctf";
    char flag[] = "flag";
    long seed;
    int bits;
    miracl *mip;
    

    /* get common data */
    fp=fopen("common.dss","rt");
    if (fp==NULL) {
        printf("file common.dss does not exist\n");
        return 0;
    }
    fscanf(fp,"%d\n",&bits);
    mip=mirsys(bits/4,16);    /* use Hex internally */
    p=mirvar(0);
    q=mirvar(0);
    g=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);
    r=mirvar(0);
    s=mirvar(0);
    k=mirvar(65535);
    hash=mirvar(0);
    tmp0=mirvar(0);
    tmp1=mirvar(0);

    innum(p,fp);
    innum(q,fp);
    innum(g,fp);
    fclose(fp);
    
    hashing(msg, strlen(msg), hash);
    fp=fopen("signed.out","rt");
    if (fp==NULL) {
        printf("file signed.out does not exist\n");
        return 0;
    }
    
    innum(r, fp);
    innum(s, fp);
    innum(k,fp);

    xgcd(k,q,k,k,k);
    
    fclose(fp);
    tmp1 = r;
    xgcd(r,q,r,r,r); // 1/r mod q
    multiply(k,s,tmp0);
    subtract(tmp0,hash,tmp0);
    multiply(r,tmp0,x);
    divide(x,q,q); // mod q
    //printf("pk: ");
    //otnum(x,stdout);

    hashing(flag, strlen(flag), hash);
    xgcd(r,q,r,r,r); // inverse the inverse of r

    xgcd(k,q,k,k,k); // 1/k mod q
    mad(x,r,hash,q,q,s);
    mad(s,k,k,q,q,s);
    //printf("flag: \n");

    FILE* fd = fopen("signed.out","wt");
    
    otnum(r,fd);
    otnum(s,fd);
    printf("Done.\n");

    return 0;
}