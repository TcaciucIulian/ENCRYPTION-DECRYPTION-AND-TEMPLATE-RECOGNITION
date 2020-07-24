#include<stdio.h>
#include<stdlib.h>
#include<math.h>

// Program realizat de Tcaciuc Iulian-Marius, grupa 143

// structura pentru 3 culori
struct pixel
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
};
// structura pentru detectie (corelatie, coordonate (x,y), culoare)
struct detectie
{
    double cor;
    int x,y;
    struct pixel cul;
};
// incarcarea unei imagini bmp la fel ca la partea 1 doar ca prin returnare cu parametrii + citire date imagine
void incarcareBMP(char* numeBmp,struct  pixel** v, unsigned char* header, unsigned int* hBMP, unsigned int* lBMP)
{
    FILE *fin;

    printf("numeBmp = %s \n", numeBmp);

    fin=fopen(numeBmp, "rb+");
    if(fin == NULL)
    {
        printf(" Nu am gasit imaginea sursa din care citesc \n");
        return ;
    }
    fseek(fin,18, SEEK_SET);
    fread(&(*lBMP), sizeof(unsigned int), 1, fin);
    fread(&(*hBMP), sizeof(unsigned int), 1, fin);

    int padding = 0;
    if((*lBMP)%4 != 0)
        padding = 4-(3*(*lBMP))%4%4 ;
    else
        padding = 0;

    fseek(fin, 0, SEEK_SET);
    fread(header, 54, 1, fin);
    *v=(struct pixel*)malloc((*lBMP)*(*hBMP)*sizeof(struct pixel));
    for(int i=(*hBMP)-1; i>=0; i--)
    {
        for(int j=0; j<(*lBMP); j++)
            fread(&(*v)[i*(*lBMP)+j], 3, 1, fin); // formula din enunt
        fseek(fin, padding, SEEK_CUR);
    }
    fclose(fin);
}
// salvarea imaginii intr-un fisier binar, la fel ca la partea 1
void salvareBMP(char* numeBmp, char* numeDestinatie, struct pixel* v, unsigned char* header, unsigned int lBMP, unsigned int hBMP)
{

    FILE* fin;
    fin=fopen(numeBmp, "rb+");
    fseek(fin, 18, SEEK_SET);
    fread(&lBMP, sizeof(unsigned int), 1, fin);
    fread(&hBMP, sizeof(unsigned int), 1, fin);
    fseek(fin, 0, SEEK_SET);
    fclose(fin);


    FILE* fout;
    fout=fopen(numeDestinatie, "wb+");

    int padding = 0;
    if(lBMP%4 != 0)
        padding = 4-(3*lBMP)%4%4 ;
    else
        padding = 0;

    fwrite(header,54,1,fout);
    for(int i=hBMP-1; i>=0; i--)
      {
      for(int j=0; j<lBMP; j++)
           {
            fwrite(&v[i*lBMP+j], 3, 1, fout); // formula enunt
           }
       fseek(fout, padding, SEEK_CUR);
      }

    fclose(fout);
}
// modificarea imaginii in pixeli gri.
void grayscaleBMP(struct pixel** v, unsigned int dimBMP)
{
    unsigned char c;
    for(int i=0; i<dimBMP; i++)
    {
        c=0.299*(*v)[i].r+0.587*(*v)[i].g+0.114*(*v)[i].b; // formula grayscale
        (*v)[i].r = c;
        (*v)[i].b = c;
        (*v)[i].g = c;
    }
}
// colorarea  imaginii prin "culoare" transmis ca parametru
void colorareBMP(struct pixel **v, int lBMP, int hBMP, struct pixel culoare, int x, int y)
{
    // formam chenarul
    for(int i=x; i<=x+14; i++)
    {
        (*v)[i*lBMP+y] = culoare;
        (*v)[i*lBMP+y+10] = culoare;
    }
    for(int i=y; i<=y+10; i++)
    {
        (*v)[x*lBMP+i] = culoare;
        (*v)[(x+14)*lBMP+i] = culoare;
    }
}
// calcularea corelatiei
double corelatie(struct pixel *vectorBMP, unsigned int lBMP, unsigned int hBMP, struct pixel *vectorSablon, double medieSGrayscale, double deviatieSablon, int x, int y)
{
    // initializari
    double sumaSGrayscale=0;
    double medieSFGrayscale;
    double sumaStandard=0;
    double deviatieFi;
    double sumaCorelatie=0;

    // sumaGrayscale si media pentru fiecare pozitie conform formulei din enunt pentur sablon
    for(int i=x; i<x+15; i++)
        for(int j=y; j<y+11; j++)
            sumaSGrayscale = sumaSGrayscale+vectorBMP[i*lBMP+j].b;
    medieSFGrayscale = sumaSGrayscale/165;

    // suna standard + deviatie conform formulei din enunt
    for(int i=x; i<x+15; i++)
        for(int j=y; j<y+11; j++)
            sumaStandard = sumaStandard+(vectorBMP[i*lBMP+j].b-medieSFGrayscale)*(vectorBMP[i*lBMP+j].b-medieSFGrayscale);
    deviatieFi = sqrt(sumaStandard/164);

    // calculare  corelatie conform formulei din proiect
    double suma1=0;
    double suma2=0;
    for(int i=x; i<x+15; i++)
        for(int j=y; j<y+11; j++)
        {
            suma1 = vectorBMP[i*lBMP+j].b-medieSFGrayscale;
            suma2 = vectorSablon[(i-x)*11+j-y].b-medieSGrayscale;
            sumaCorelatie = sumaCorelatie+(suma1*suma2)/(deviatieFi*deviatieSablon);
        }
    // sumaCorelatia
    sumaCorelatie = sumaCorelatie/165;
    return sumaCorelatie;
}
// Functai de templatematching unde se creeaza imaginea "numeDetecie"
void templatematching(char* numeBmp, char* numeDetectie, char* numeSablon,unsigned int lBMP, unsigned int hBMP, struct pixel culoare, struct detectie **v, int *dimBMP, float *pragS)
{
    // initializari
    struct  pixel *vectorBMP;
    struct  pixel *vectorSablon;
    struct  pixel *detect;
    double sumaSGrayscale=0;
    double sumaStandard=0;
    double medieSGrayscale=0;
    unsigned char header[54];
    unsigned char header1[54];

    // memorare in vectori
    incarcareBMP(numeSablon,&vectorSablon,header1,&hBMP,&lBMP);
    incarcareBMP(numeBmp,&vectorBMP,header,&hBMP,&lBMP);
    incarcareBMP(numeDetectie,&detect,header,&hBMP,&lBMP);

    // transformarea in imagini grayscale
    grayscaleBMP(&vectorBMP,lBMP*hBMP);
    grayscaleBMP(&vectorSablon,11*15);

    // suma sablon mic
    for(int i=0; i<15; i++)
        for(int j=0; j<11; j++)
            sumaSGrayscale = sumaSGrayscale+vectorSablon[i*11+j].b;
    medieSGrayscale = (double)sumaSGrayscale/165;

    // deviatie sablon mic
    for(int i=0; i<15; i++)
        for(int j=0; j<11; j++)
            sumaStandard = sumaStandard+(vectorSablon[i*11+j].b-medieSGrayscale)*(vectorSablon[i*11+j].b-medieSGrayscale);
    double deviatiaStandard = sqrt(sumaStandard/164);

    // creare vector detectii  liniarizat colorat
    for(int i=0; i<hBMP-15; i++)
        for(int j=0; j<lBMP-11; j++)
        {
            if(corelatie(vectorBMP, lBMP, hBMP, vectorSablon, medieSGrayscale, deviatiaStandard, i, j) >= *pragS)
            {
                // coloram chenarele
                colorareBMP(&detect, lBMP, hBMP, culoare, i, j);
                // memoram in vector de corelatii
                (*dimBMP)++;
                (*v) = realloc(*v,(*dimBMP)*sizeof(struct detectie));
                (*v)[*dimBMP-1].cor = corelatie(vectorBMP, lBMP, hBMP, vectorSablon, medieSGrayscale, deviatiaStandard, i, j);
                (*v)[*dimBMP-1].x = i;
                (*v)[*dimBMP-1].y = j;
                (*v)[*dimBMP-1].cul = culoare;
            }
        }
    // salvarea vectorului colorat  de detectii "detect" in fisierul binar.
    salvareBMP(numeDetectie, numeDetectie, detect, header, lBMP, hBMP);
}
// Functia de comparare necesara pentru qsort
int cmp(const void* a, const void* b)
{
    struct detectie va = *(struct detectie *)a;
    struct detectie vb = *(struct detectie *)b;
    if(va.cor > vb.cor)
        return -1;
    if(va.cor < vb.cor)
        return 1;
    return 0;
}
// Functie care verifica daca 2 imagini se suprapun
int supraScriere(struct detectie a, struct  detectie b)
{
    // formula luata de pe geeksforgeeks - > https://www.geeksforgeeks.org/find-two-rectangles-overlap/
    if(a.x>b.x+14 || b.x>a.x+14)
        return 0;
    if(a.y>b.y+10 || b.y>a.y+10)
        return 0;
    return 1;
}
// Funcite minim dintre 2 numere
int minim(int x, int y)
{
    if(x > y)
        return y;
    return x;
}
// Functie maxim dintre 2 numere
int maxim(int x, int y)
{
    if(x > y)
        return x;
    return y;
}
// Calcularea ariei intersectiei a doua imagini.
double arie(struct detectie a, struct detectie b)
{
    double arieIntersectie = (minim(a.x+14, b.x+14)-maxim(a.x, b.x)+1)*(minim(a.y+10, b.y+10)-maxim(a.y, b.y)+1);
    return arieIntersectie/(165+165-arieIntersectie);
}
// Functie pentru eliminarea nonmaximelor
void eliminareNonmaxime(struct detectie *v,int dimBMP)
{
for(int i=0; i<dimBMP; i++)
        for(int j=i+1; j<dimBMP; j++)
            if(supraScriere(v[i], v[j]) == 1)
            {
                if(arie(v[i],v[j]) >= 0.2)
                {
                    //eliminam din vector
                    dimBMP--;
                    for(int k=j; k<dimBMP; k++)
                        v[k] = v[k+1];
                    j--;
                }
            }
}
int main()
{
    // Initializari
    unsigned char header[54];
    char test[]="test.bmp";
    char bmpFinal[]="bmpFinal.bmp";
    char testDetectie[]="testDetectie.bmp";
    char cifra0[]="cifra0.bmp";
    char cifra1[]="cifra1.bmp";
    char cifra2[]="cifra2.bmp";
    char cifra3[]="cifra3.bmp";
    char cifra4[]="cifra4.bmp";
    char cifra5[]="cifra5.bmp";
    char cifra6[]="cifra6.bmp";
    char cifra7[]="cifra7.bmp";
    char cifra8[]="cifra8.bmp";
    char cifra9[]="cifra9.bmp";
    struct pixel* bmp;
    unsigned int hBMP=0;
    unsigned int lBMP=0;
    // incarcare si salvare

    incarcareBMP("test.bmp", &bmp, header, &hBMP, &lBMP);
    salvareBMP("test.bmp","testDetectie.bmp", bmp, header, lBMP, hBMP);
    printf("%u %u", lBMP, hBMP);

    int dimBMP = 0;
    float pragS;
    printf("\n Citim pragul : ");
    scanf("%f",&pragS);
    printf("%f \n",pragS);
    struct detectie* v = malloc(sizeof(struct detectie));

    // Desenarea ferestrelor prin culorile date in enunt
    templatematching(test, testDetectie, cifra0, lBMP, hBMP, (struct pixel){0,0,255}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra1, lBMP, hBMP, (struct pixel){0,255,255}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra2, lBMP, hBMP, (struct pixel){0,255,0}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra3, lBMP, hBMP, (struct pixel){255,255,0}, &v,&dimBMP, &pragS);
    templatematching(test, testDetectie, cifra4, lBMP, hBMP, (struct pixel){255,0,255}, &v,&dimBMP, &pragS);
    templatematching(test, testDetectie, cifra5, lBMP, hBMP, (struct pixel){255,0,0}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra6, lBMP, hBMP, (struct pixel){192,192,192}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra7, lBMP, hBMP, (struct pixel){0,140,255}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra8, lBMP, hBMP,(struct pixel){128,0,128}, &v, &dimBMP, &pragS);
    templatematching(test, testDetectie, cifra9, lBMP, hBMP, (struct pixel){0,0,128}, &v, &dimBMP, &pragS);

    // sortare descrescatoare a vectorului de detectii
    qsort(v,dimBMP, sizeof(struct detectie), cmp);

    eliminareNonmaxime(v,dimBMP);

    // coloare si salvare  imagine finala dupa eliminarea nonmaxilor
    v = realloc(v,dimBMP*sizeof(struct detectie));
    for(int i=0; i<dimBMP; i++)
        colorareBMP(&bmp, lBMP, hBMP, v[i].cul, v[i].x, v[i].y);
    salvareBMP(bmpFinal,bmpFinal, bmp, header, lBMP, hBMP);

    return 0;
}
