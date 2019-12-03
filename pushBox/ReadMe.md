## Challenge

This challenge uses the open source library [MIRACL](https://github.com/miracl/MIRACL) to implement a vulnerable DSA license verification for the pushBox game. The function `debug` can be used to generate flag and has been left in the binary to all the gamers. The `debug` function basically does the following things:

- Generate a new random common.dss which contains p, q and g
- Generate a new random public.dss and private.dss which contains public and private keys
- Generate a random `k`
- Use the generated `k` to sign message `flag`, print out part of the <r,s> pair as the flag
- Use the same `k` to sign message `d3ctf`, write <r,s> to the signature file `signed.out`
- Write `k` to the signature file `signed.out`, remove `private.dss`

Upon start of the game, the program will check if the license is correct (if the signed signature is the valid signature for the input name), if so then the game will be started, besides, if the verified name is `flag`, the program will print out part of <r,s> as the flag, which means that we need to recover the signature for the message `flag` to win this challenge.

Then it comes to the easy part, according to the leaked random `k` and a given signature,  we should be very easy to obtain the private key `x`:
$$
x \equiv r^{-1} (ks-H(m))\ mod\ q
$$
After that, we can use the leaked k and recovered pk to sign `flag` ourselves to solve this challenge.

## Solution

```c
#include <stdio.h>
#include <memory.h>
#include <miracl/miracl.h>
#include <stdlib.h>
#include <string.h>

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

int main(){
    FILE *fp;
    big p,q,g,x,y,r,s,k,hash,tmp0,tmp1;
    char msg[] = "d3ctf";
    char flag[] = "flag";
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

    hashing(flag, strlen(flag), hash);
    xgcd(r,q,r,r,r); // inverse the inverse of r

    xgcd(k,q,k,k,k); // 1/k mod q
    mad(x,r,hash,q,q,s);
    mad(s,k,k,q,q,s);

    FILE* fd = fopen("signed.out","wt");
    
    otnum(r,fd);
    otnum(s,fd);
    printf("Done.\n");

    return 0;
}
```

