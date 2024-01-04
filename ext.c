#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#include <stdlib.h>

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);//OK
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);//OK
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
        char *nombre);//OK
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);//OK
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
        char *nombreantiguo, char *nombrenuevo);//OK
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
        EXT_DATOS *memdatos, char *nombre);//OK
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
        EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
        char *nombre,  FILE *fich);//OK
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
        EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
        EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);
int numeroFichero(EXT_ENTRADA_DIR *directorio, char* nombre); //OK ANADIDO
int existeFichero(EXT_ENTRADA_DIR *directorio, char* nombre); // OK AÑADIDO

void quitarSaltoLinea(char* palabra);

int main()
{
    char *comando=(char*)malloc(LONGITUD_COMANDO);
    char *orden=(char*)malloc(LONGITUD_COMANDO);
    char *argumento1=(char*)malloc(LONGITUD_COMANDO);
    char *argumento2=(char*)malloc(LONGITUD_COMANDO);
    
    int i,j;
    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;
    
    // Lectura del fichero completo de una sola vez
    //...
    fent = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
    
    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    
    // Buce de tratamiento de comandos
    for (;;){
        do {
            printf (">> ");
            fflush(stdin);
            fgets(comando, LONGITUD_COMANDO, stdin);
        } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);

        printf("\n");

        if (strcmp(orden,"info")==0) {
            LeeSuperBloque(&ext_superblock);
        }else if (strcmp(orden,"bytemaps")==0) {
            Printbytemaps(&ext_bytemaps);
        }else if (strcmp(orden,"dir")==0) {
            Directorio(directorio, &ext_blq_inodos);
        }else if (strcmp(orden,"rename")==0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
        }else if (strcmp(orden,"imprimir")==0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        }else if (strcmp(orden,"remove")==0) {
            Borrar(directorio, &ext_blq_inodos,&ext_bytemaps, &ext_superblock,argumento1, fent);
        }else if (strcmp(orden,"copy")==0) {
            Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock,
            memdatos,argumento1, argumento2,  fent);
        }else if (strcmp(orden,"salir")==0) {
            printf("SALIENDO...\n");
            return 0;
        }
        /*//...
        // Escritura de metadatos en comandos rename, remove, copy     
        Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
        GrabarByteMaps(&ext_bytemaps,fent);
        GrabarSuperBloque(&ext_superblock,fent);
        if (grabardatos)
        GrabarDatos(memdatos,fent);
        grabardatos = 0;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden,"salir")==0){
        GrabarDatos(memdatos,fent);
        fclose(fent);
        return 0;
        }*/
        printf("\n");
    }
}

void quitarSaltoLinea(char* palabra){
    int contador=0;
    if(palabra!=NULL){
        while(palabra[contador]!='\0'){
            if(palabra[contador]=='\n'){
                palabra[contador]='\0';
            }
            contador++;
        }
    }
   
}


int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
    int error = 1 ;
    char *aux;

    if(strcomando!=NULL){

        aux = strtok(strcomando," ");
        quitarSaltoLinea(aux);
        strcpy(orden,aux);

        aux = strtok(NULL," ");
        if(aux!=NULL){
            quitarSaltoLinea(aux);
            strcpy(argumento1,aux);
        }else{
            argumento1=NULL;
        }

        aux = strtok(NULL," ");
        if(aux!=NULL){
            quitarSaltoLinea(aux);
            strcpy(argumento2,aux);
        }else{
            argumento2=NULL;
        }
    }

    if(   (strcmp(orden,"rename") == 0) || (strcmp(orden,"copy") == 0) ){
        if(argumento1 ==NULL || argumento2 == NULL){
            printf("INSUFICIENTES ARGUMENTOS\n");
        }else{
            error=0;
        }
    }else if((strcmp(orden,"imprimir") == 0) || (strcmp(orden,"remove") == 0)){
        if( argumento2 != NULL){
            printf("DEMASIADOS ARGUMENTOS\n");
        }else{
            error=0;
        }
    }else if((strcmp(orden,"dir") == 0)||(strcmp(orden,"info") == 0) || (strcmp(orden,"bytemaps") == 0 ) || (strcmp(orden,"salir") == 0)){
        if(argumento1 !=NULL || argumento2 != NULL){
            printf("DEMASIADOS ARGUMENTOS\n");
        }else{
            error=0;
        }
    }else{
        printf("COMANDO NO EXISTE\n");
    }
    return error;
}


void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
    printf("Bloque = %d Bytes\n", psup->s_block_size);
    printf("Inodos particion = %d\n", psup->s_inodes_count );
    printf("Inodos libres = %d\n", psup->s_free_inodes_count );
    printf("Bloques particion = %d\n", psup->s_blocks_count );
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}


void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){

    printf("Inodos: ");
    for(int i=0; i<25; i++){
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\nBloques [0-25]: ");
    for(int i=0; i<25; i++){
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}


void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){

    for(int i=1; i<MAX_FICHEROS; i++){
        if(directorio[i].dir_inodo!=0xFFFF){
        printf("%s" ,directorio[i].dir_nfich);
        
        printf("\tTamaño: %d ", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
        printf("\tInodo:%d ",directorio[i].dir_inodo);
        printf("\tBloques: ");
        for(int x=0; x<MAX_NUMS_BLOQUE_INODO; x++){
            if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[x] != 0xFFFF){
            printf("%d ",inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[x]);
            }
        }
        printf("\n");
        }
    }

}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
    
    int error=0;
    int contador=0;

    for(int i=1; i<MAX_FICHEROS;i++){
        //printf("%s, %s\n", nombreantiguo,directorio[i].dir_nfich);
        if(strcmp(directorio[i].dir_nfich,nombreantiguo)==0){
            contador=i;
            error=1;
            for(int t=1; t<MAX_FICHEROS; t++){
                if(strcmp(directorio[t].dir_nfich,nombrenuevo)==0){
                    error=0;
                    t=MAX_FICHEROS;
                }
            }
            i=MAX_FICHEROS;
        }
    }

    if(error==1){
        strcpy(directorio[contador].dir_nfich,nombrenuevo);
        printf("EL FICHERO %s SE CAMBIO DE NOMBRE A %s\n", nombreantiguo, nombrenuevo);
    }else{
        printf("ERROR EN LOS NOMBRES\n");
    }
    return error;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
    
    if(existeFichero(directorio, nombre)==1){
        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            if(inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[i]!=0xFFFF){
                printf("%s\n",memdatos[inodos->blq_inodos[directorio[BuscaFich(directorio,NULL, nombre)].dir_inodo].i_nbloque[i]-4].dato);   
                printf("%d\n ",inodos->blq_inodos[directorio[BuscaFich(directorio,NULL, nombre)].dir_inodo].i_nbloque[i]-4);
            }
        }
    }else{
        printf("EL FICHERO NO EXITE\n");
    }
}

int existeFichero(EXT_ENTRADA_DIR *directorio, char* nombre){
    int error = 0;
    for(int i=1; i<MAX_FICHEROS;i++){
        if(strcmp(directorio[i].dir_nfich,nombre)==0){
            error=1;
        }
    }
    return error;
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
    int fichero = -1;
    for(int i=1; i<MAX_FICHEROS;i++){
        if(strcmp(directorio[i].dir_nfich,nombre)==0){
            fichero=i;
        }
    }
    return fichero;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
    int error=0;
    if(existeFichero(directorio, nombre)==1){
        ext_bytemaps->bmap_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo] =0;
        ext_superblock->s_free_inodes_count++;
        for(int x=0; x<MAX_NUMS_BLOQUE_INODO; x++){
            if(inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[x] != 0xFFFF){
                ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[x]]=0;
                ext_superblock->s_free_blocks_count++;
            }
        }
        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[i]=0xFFFF;
        }
        inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].size_fichero=0;
        directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo=0xFFFF;
        strcpy(directorio[BuscaFich(directorio, NULL,nombre)].dir_nfich,"");
        printf("SE ELIMINO EL FICHERO %s\n", nombre);
    }else{
        printf("NO EXISTE EL FICHERO  %s\n", nombre);
        error=1;
    }

    return error;

}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){

    int contador=0;
    int num_fichero;

    if(existeFichero(directorio, nombreorigen)==1){

        if(existeFichero(directorio, nombredestino)==1){
            Borrar(directorio,inodos,ext_bytemaps, ext_superblock, nombredestino, fich);
        }else{
            printf("SE CREO EL FICHERO %s\n", nombredestino);
        }


        for(int i=1; i<MAX_FICHEROS;i++){
                if(directorio[i].dir_inodo==0xFFFF){
                    for(int t=0; t<MAX_INODOS; t++){
                        if(ext_bytemaps->bmap_inodos[t]==0){
                            directorio[i].dir_inodo=t;
                            strcpy(directorio[i].dir_nfich,nombredestino);
                            ext_bytemaps->bmap_inodos[t]=1;
                            ext_superblock->s_free_inodes_count--;
                            i=MAX_FICHEROS;
                            t=MAX_INODOS;
                        }
                    }
                }
        }
        inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].size_fichero=inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombreorigen)].dir_inodo].size_fichero;

        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            if(inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombreorigen)].dir_inodo].i_nbloque[i]!=0xFFFF){
                for(int t=0; t<MAX_BLOQUES_DATOS;t++){
                    if(ext_bytemaps->bmap_bloques[t]==0){
                        for(int y=0;y<MAX_NUMS_BLOQUE_INODO;y++){
                            if(inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].i_nbloque[y]==0xFFFF){
                                inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].i_nbloque[y]=t;
                                strcpy(memdatos[t-4].dato , memdatos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo-4].dato);
                                ext_superblock->s_free_blocks_count--;
                                y=MAX_NUMS_BLOQUE_INODO;
                            }
                        }
                        ext_bytemaps->bmap_bloques[t]=1;
                        t=MAX_BLOQUES_DATOS;
                    }
                }
            }
        }
                   
        printf("SE COPIARON LOS DATOS DE %s A %s\n",nombreorigen, nombredestino);
    }else{
        printf("EL FICHERO ORIGEN NO EXISTE\n");
    }

}



}

