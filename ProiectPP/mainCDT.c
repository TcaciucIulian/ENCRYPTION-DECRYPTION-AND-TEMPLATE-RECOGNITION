 #include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Program realizat de Tcaciuc Iulian-Marius , grupa 143

// Structura pentru pixel[3]
struct pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};
//Structura pentru memorarea caracteristicilor unei imaginii + vectorul p folosit pentru liniarizare
struct bmp
{
    struct pixel *p;
    unsigned int hBMP;
    unsigned int lBMP;
    unsigned char *header;
    unsigned int padding;
};
// Functia xorshift32 pentru numere pseudo aleatoare.
uint32_t xorshift32(uint32_t state[static 1])
{
    uint32_t x = state[0];
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state[0] = x;
    return x;
}
// Functie pentru incarcarea unei imagini. Aici citim si caracteristicile imaginii + liniarizare. (2)
struct bmp incarcareBMP(char* numeBmp)
{
    // deschidere fisier initial, din care citim + initializari.
    FILE *fin;
    unsigned int dimBmp;
    printf("numeBmp = %s \n", numeBmp);
    fin=fopen(numeBmp, "rb+");
    if(fin == NULL)
    {
        printf(" Nu am gasit imaginea sursa din care citesc \n");
        return ;
    }

    //citirea caractersiticilor unei imagini (dimBmp, lungime, latime, header, padding)
    struct bmp *v;
    v=(struct bmp*)malloc(sizeof(struct bmp));
    fseek(fin, 2, SEEK_SET);
    fread(&dimBmp, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in octeti: %u\n", dimBmp);

    fseek(fin, 18, SEEK_SET);
    fread(&v->lBMP, sizeof(unsigned int), 1, fin);
    fread(&v->hBMP, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in pixeli: %u x %u\n", v->hBMP, v->lBMP);

    fseek(fin, 0, SEEK_SET);
    v->header=(struct bmp*)malloc(54*sizeof(unsigned char));
    fread(v->header, 54, 1, fin);
    if(v->lBMP%4 != 0)
        v->padding = 4-(3*v->lBMP) % 4 % 4 ;
    else
        v->padding = 0;

    // liniarizarea imaginii in vectorul de tip pixel p.
    struct pixel x;
    v->p=(struct pixel*)malloc(v->lBMP*v->hBMP*sizeof(struct pixel));
    int k=0;
    fseek(fin, 54, SEEK_SET);
    for(int i=v->hBMP-1; i>=0; i--)
    {
        for(int j=0; j<v->lBMP; j++)
        {
            fread(&x, 3, 1, fin);
            v->p[k] = x;
            k++;
        }
    }
    fclose(fin);
    return *v;
}
// Functie pentru salvarea unei imaginii. (3)
struct bmp salvareBMP(char* numeBmp, char* numeDestinatie)
{
    //initializare v.
    struct bmp v = incarcareBMP(numeBmp);

    //deschidere fisierul in care salvam imaginea.
    FILE *fout = fopen(numeDestinatie, "wb+");

    // salvarea imaginii (scriere header + fiecare 3 pixeli in fisierul binar nume Destinatie)
    fseek(fout, 0, SEEK_SET);
    fwrite(v.header, 54, 1, fout);

    fseek(fout, 54, SEEK_SET);
    int numarElemente=0;
    for(int i=0; i<v.hBMP; i++)
    {
        for(int j=0; j<v.lBMP; j++)
        {
            fwrite(&v.p[numarElemente], 3, 1, fout);
            numarElemente++;
        }
        fseek(fout, v.padding, SEEK_CUR);
    }
    fclose(fout);
    return v;
}
// Functie pentru calcularea "xor" intre 2 pixeli prin formula explicata in enunt.
struct pixel xoor1(struct pixel y, struct pixel z)
{
    struct pixel c;
    (c.r) = (y.r) ^ (z.r);
    (c.g) = (y.g) ^ (z.g);
    (c.b) = (y.b) ^ (z.b);
    return c;
}
// Functie pentru calcularea "xor" intre un intreg si un pixel prin forumla explicata in enunt ( ne deplasam la dreapta ca sa calculam octetii folosind masca 1111 1111 )
struct pixel xoor2(int x, struct pixel y)
{
    struct pixel c;
    (c.b) = (y.b) ^ (x&255);
    (c.g) = (y.g) ^ ((x>>8)&255);
    (c.r) = (y.r) ^ ((x>>16)&255);
    return c;
}
// Functie pentur criptarea unei imaginii. Imaginea criptata este salvata in vectorul criptat si afisat in "bmpCriptat".
struct pixel* criptare(char *numeBmp, char *bmpCriptat, char *secretKey)
{
    // Initiazilare imagine initiala
    struct bmp v=incarcareBMP(numeBmp);

    // struct bmp v=salvareBMP(numeBmp,bmpCriptat);

    // Deschidere fisier secretKey unde avem r0 si sv necesare pentru criptare.
    FILE *secret=fopen(secretKey, "r");
    int r0 = 0;
    int sv = 0;
    fscanf(secret,"%d", &r0);
    fscanf(secret,"%d", &sv);
    printf("\n %d %d \n", r0, sv);

    // Memorarea a n numere pseudo aleatoare necesare pentru permutare.
    int n = v.lBMP * v.hBMP * 2-1;
    uint32_t *xor=(uint32_t *)malloc(n*sizeof(uint32_t));
    uint32_t state[0];
    xor[0] = r0;
    state[0] = r0;
    for(int i=1; i<n; i++)
    {
        xor[i] = xorshift32(state);
        state[0] = xor[i];
    }

    // Initiazilare permutare initiala.
    int *a;
    a=(int *)malloc(v.lBMP*v.hBMP*sizeof(int));
    for(int i=0; i<v.hBMP*v.lBMP; i++)
        a[i] = i;
    // for(int i=1;i<5;i++)
    //    printf("%d ",a[i]);

    // Permutare aleatoare folosind algoritmul lui Durstenfeld si numerele pseudo aleatoare
    printf(" \n ");
    int j;
    int k = 0;
    int aux;
    for(int i=v.hBMP*v.lBMP-1; i>0; i--)
    {
        j = xor[k]%(i+1);
        k++;
        aux = a[j];
        a[j] = a[i];
        a[i] = aux;
    }
    // for(int i=0;i<5;i++)
    //    printf("%d ",a[i]);


    // aici permutat pixelii conform permutarii din vectorul a.
    struct pixel *bmpf;
    bmpf=(struct pixel*)malloc(v.lBMP*v.hBMP*sizeof(struct pixel));
    for(int i=0; i<v.lBMP*v.hBMP; i++)
    {
        bmpf[a[i]] = v.p[i];
    }

    // aici se realizeaza criptarea conform formulei din enuntul proiectului.
    struct pixel *criptat;
    criptat=(struct pixel*)malloc(v.lBMP*v.hBMP*sizeof(struct pixel));
    k=0;
    for(int i=0; i<v.lBMP*v.hBMP; i++)
    {
        if(k==0)
        {
            // aplicam formula din enunt pentru k=0;
            struct pixel c = xoor2(sv, bmpf[0]);
            criptat[k] = xoor2(xor[v.lBMP*v.hBMP], c);
        }
        else
        {
            // aplicam formula din enunt pentru k>0;
            struct pixel c = xoor1(criptat[k-1], bmpf[k]);
            criptat[k] = xoor2(xor[v.lBMP*v.hBMP+k], c);
        }
        k++;
    }

    // salvarea imaginii in bmp Criptat.

    // deschidem fisierul pentru scriere. Afisam header iar apoi vectorul criptat in fisierul binar bmpCriptat + deplasare padding
    FILE *fout=fopen(bmpCriptat, "wb+");
    fseek(fout, 0, SEEK_SET);
    fwrite(v.header, 54, 1, fout);
    fseek(fout, 54, SEEK_SET);
    int numarElemente=0;
    for(int i=0; i<v.hBMP; i++)
    {
        for(int j=0; j<v.lBMP; j++)
        {
            fwrite(&criptat[numarElemente], 3, 1, fout);
            numarElemente++;
        }
        fseek(fout, v.padding, SEEK_CUR);
    }
    fseek(fout, 0, SEEK_SET);
    fwrite(v.header, 54, 1, fout);
    fclose(fout);
    return criptat;
}

void decriptare(char *numeBmp, char *bmpCriptat, char *bmpDecriptat, char *secretKey)
{
    // deschidem fisierul binar pentru scrierea imaginii decriptate
    FILE *fout = fopen(bmpDecriptat, "wb+");

    //initializare vector caractersitici si vectorul cu imaginea criptata liniarizata
    struct bmp v = salvareBMP(numeBmp, bmpCriptat);
    struct pixel *criptat = criptare(numeBmp, bmpCriptat, secretKey);

    // citire valori pentru decriptare ( aceleasi valori ca la criptare)

    FILE *secret = fopen(secretKey, "r");
    int r0 = 0;
    int sv = 0;
    fscanf(secret, "%d", &r0);
    fscanf(secret, "%d", &sv);
    printf("\n %d %d \n", r0, sv);

    // generam numere pseudo aleatoare pornind de la valoare r0
    int n = v.lBMP*v.hBMP*2-1;
    uint32_t *xor=(uint32_t *)malloc(n*sizeof(uint32_t));
    uint32_t state[0];
    xor[0] = r0;
    state[0] = r0;
    for(int i=1; i<n; i++)
    {
        xor[i] = xorshift32(state);
        state[0] = xor[i];
    }

    // initializare permutare
    int *a;
    a=(int *)malloc(v.lBMP*v.hBMP*sizeof(int));
    for(int i=0; i<v.hBMP*v.lBMP; i++)
        a[i] = i;
    // for(int i=1;i<5;i++)
    // printf("%d ",a[i]);

    printf(" \n ");
    // permutare aleatoare folosing algoritmul lui Durstenfeld si numerele pseudo aleaotare
    int j;
    int k = 0;
    int aux;
    for(int i=v.hBMP*v.lBMP-1; i>0; i--)
    {
        j = xor[k]%(i+1);
        k++;
        aux = a[j];
        a[j] = a[i];
        a[i] = aux;
    }


    // calcularea inversei permutarii a.
    int *l;
    l=(int *)malloc(v.lBMP*v.hBMP*sizeof(int));
    for(int i=0; i<v.hBMP*v.lBMP; i++)
        l[i] = i;
    for(int i=v.hBMP*v.lBMP-1; i>0; i--)
    {
        l[a[i]] = i;
    }

    // folosim inversa relatiei de substitutie folosita in procesul de criptare conform formulei din enunt
    struct pixel *criptat1;
    criptat1=(struct pixel*)malloc(v.lBMP*v.hBMP*sizeof(struct pixel));
    k = 0;
    for(int i=0; i<v.lBMP*v.hBMP; i++)
    {
        if(k == 0)
        {
            struct pixel c = xoor2(sv, criptat[0]);
            criptat1[k] = xoor2(xor[v.lBMP*v.hBMP], c);
        }
        else
        {
            struct pixel c = xoor1(criptat[k-1], criptat[k]);
            criptat1[k] = xoor2(xor[v.lBMP*v.hBMP+k], c);
        }
        k++;
    }

    // initializam vectorul pentru decriptare
    struct pixel *decriptat;
    decriptat=(struct pixel*)malloc(v.lBMP*v.hBMP*sizeof(struct pixel));

    // memoram conform formulei din enunt
    for(int i=0; i<v.lBMP*v.hBMP; i++)
        decriptat[l[i]] = criptat1[i];

    // salvarea liniarizarii din decriptat in bmpDecriptat.
    fseek(fout, 0, SEEK_SET);
    fwrite(v.header, 54, 1, fout);
    fseek(fout, 54, SEEK_SET);
    int numarElemente=0;
    for(int i=0; i<v.hBMP; i++)
    {
        for(int j=0; j<v.lBMP; j++)
        {
            fwrite(&decriptat[numarElemente], 3,1 ,fout);
            numarElemente++;
        }
        fseek(fout,v.padding,SEEK_CUR);
    }
    fseek(fout,0,SEEK_SET);
    fwrite(v.header,54,1,fout);
    fclose(fout);
    fclose(secret);
}

void test(char *numeBmp, char *bmpCriptat, char *secretKey)
{
    // initializare valori
    struct bmp v = salvareBMP(numeBmp, bmpCriptat);
    criptare(numeBmp, bmpCriptat, secretKey);
    FILE *criptat = fopen(bmpCriptat, "rb+");


    double fb; // frecventa estimata teoretic -> formula enunt proiect
    fb = (v.lBMP*v.hBMP)/256.0;

    // calculam frecventa valorii i pe fiecare canal de culoare
    unsigned int  *frecvb=(unsigned int*)calloc(256,sizeof(unsigned int));
    unsigned int  *frecvg=(unsigned int*)calloc(256,sizeof(unsigned int));
    unsigned int  *frecvr=(unsigned int*)calloc(256,sizeof(unsigned int));
    unsigned char c;
    for(int i=0; i<v.hBMP*v.lBMP; i++)
    {
        fread(&c, 1, 1, criptat);
        frecvb[c]++;
        fread(&c, 1, 1, criptat);
        frecvg[c]++;
        fread(&c, 1, 1, criptat);
        frecvr[c]++;
    }

    // realizam testul x^2 conform formulei din enunt pe fiecare canal de culoare
    double sb=0;
    double sg=0;
    double sr=0;
    for(int i=0; i<256; i++)
    {
        sb = sb+((frecvb[i]-fb)*(frecvb[i]-fb))/fb;
        sg = sg+((frecvg[i]-fb)*(frecvg[i]-fb))/fb;
        sr = sr+((frecvr[i]-fb)*(frecvr[i]-fb))/fb;
    }
    printf("\n Valori test : r: %f\n g: %f\n b: %f \n",sr,sg,sb);
}
int main()
{
    char numeBmp[] = "peppers.bmp";
    //  char numeDestinatie[]="peppers2.bmp";
    char bmpCriptat[] = "bmpCriptat.bmp";
    char secretKey[] = "secret_key.txt";
    char bmpDecriptat[] = "bmpDecriptat.bmp";
    // salvareBMP(numeBmp, numeDestinatie);
    // criptare(numeBmp, bmpCriptat, secretKey);
    // decriptare(numeBmp, bmpCriptat, bmpDecriptat, secretKey);
    test(numeBmp, bmpCriptat, secretKey);
    decriptare(numeBmp, bmpCriptat, bmpDecriptat, secretKey);
    return 0;
}
