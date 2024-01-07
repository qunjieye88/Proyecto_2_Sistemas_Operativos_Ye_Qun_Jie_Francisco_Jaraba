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
        EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);//OK
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

    fclose(fent);
    
    // Buce de tratamiento de comandos
    for (;;){
        //RECOGEMOS EL COMANDO Y LO DIVIDIMOS
        do {
            printf (">> ");
            fflush(stdin);
            fgets(comando, LONGITUD_COMANDO, stdin);
        } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);

        printf("\n");
        //DEPENDIENDO DE LA ORDEN SE ASIGNARA UNA FUNCION DETERMINADO
        if (strcmp(orden,"info")==0) {
            LeeSuperBloque(&ext_superblock);
        }else if (strcmp(orden,"bytemaps")==0) {
            Printbytemaps(&ext_bytemaps);
        }else if (strcmp(orden,"dir")==0) {
            Directorio(directorio, &ext_blq_inodos);
        }else if (strcmp(orden,"rename")==0) {
            grabardatos=Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
        }else if (strcmp(orden,"imprimir")==0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        }else if (strcmp(orden,"remove")==0) {
            grabardatos=Borrar(directorio, &ext_blq_inodos,&ext_bytemaps, &ext_superblock,argumento1, fent);
        }else if (strcmp(orden,"copy")==0) {
            grabardatos=Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock,
            memdatos,argumento1, argumento2,  fent);
        }else if (strcmp(orden,"salir")==0) {//SALIMOS DEL PROGRAMA
            //...
            // Escritura de metadatos en comandos rename, remove, copy     
            //SI GRABAR DATOS ES 1, SE ACTUALIA EL FICHERO
            if (grabardatos==1){
                fent = fopen("particion.bin","w");
                GrabarSuperBloque(&ext_superblock,fent);
                GrabarByteMaps(&ext_bytemaps,fent);
                Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
                GrabarDatos(memdatos,fent);
                grabardatos = 0;
                fclose(fent);
            }
            //Si el comando es salir se habrán escrito todos los metadatos
            //faltan los datos y cerrar

            
            printf("SALIENDO...\n");
            return 0;
        }

        
    }
}

//FUNCION QUE ELIMINA EL SALTO DE LINEA Y LO CONVIERTE EN '\0'
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

//FUNCION QUE SEPARA EL COMANDO EN ORDEN, ARGUMENTO1 Y ARGUMENTO2, DEVUELVE 1 SI HAY ERROR Y 0 SI ESTA CORRECTO
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
    int error = 1 ;
    char *aux;
//SEPARA EL COMANDO EN 3 PARTES
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
//COMPRUEBA SI LOS ARGUMENTOS Y LA ORDEN NO TIENE ERRORES
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

//IMPRIME EL SUPERBLOQUE
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
    printf("Bloque = %d Bytes\n", psup->s_block_size);
    printf("Inodos particion = %d\n", psup->s_inodes_count );
    printf("Inodos libres = %d\n", psup->s_free_inodes_count );
    printf("Bloques particion = %d\n", psup->s_blocks_count );
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

//IMPRIME EL BYTEMAPS
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){

    printf("Inodos: ");
    for(int i=0; i<MAX_INODOS; i++){
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\nBloques [0-25]: ");
    for(int i=0; i<25; i++){
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

//IMPRIME EL DIRECTORIO
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){

    for(int i=1; i<MAX_FICHEROS; i++){
        if(directorio[i].dir_inodo!=0xFFFF){
        //IMPRIME EL NOMBRE, TAMAÑO Y EL INODO DEL FICHERO
        printf("%s" ,directorio[i].dir_nfich);
        printf("\tTamaño: %d ", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
        printf("\tInodo:%d ",directorio[i].dir_inodo);
        printf("\tBloques: ");
        //DE CADA FICHERO IMPRIME LA POSICION DEL BLOQUE
        for(int x=0; x<MAX_NUMS_BLOQUE_INODO; x++){
            if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[x] != 0xFFFF){
            printf("%d ",inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[x]);
            }
        }
        printf("\n");
        }
    }

}
//CAMBIA EL NOMBRE DEL FICHERO
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
    
    int correcto=0;
    int contador=0;
    // COMPRUEBA SI EL NOMBRE ANTUGUO EXISTE O NO, SI EXISTE, SEGUARDA EN ERROR UN 1
    for(int i=1; i<MAX_FICHEROS;i++){
        if(strcmp(directorio[i].dir_nfich,nombreantiguo)==0){
            contador=i;
            correcto=1;
            //COMPRUEBA SI EL NOMBRE NUEVO EXISTE O NO, SI EXISTE, SEGUARDA EN ERROR UN 0
            for(int t=1; t<MAX_FICHEROS; t++){
                if(strcmp(directorio[t].dir_nfich,nombrenuevo)==0){
                    correcto=0;
                    t=MAX_FICHEROS;
                }
            }
            i=MAX_FICHEROS;
        }
    }
    //SI LOS NOMBRES ESTAN CORRECTOS, EL NOMBRE SE CAMBIA
    if(correcto==1){
        strcpy(directorio[contador].dir_nfich,nombrenuevo);
        printf("EL FICHERO %s SE CAMBIO DE NOMBRE A %s\n", nombreantiguo, nombrenuevo);
    }else{
        printf("ERROR EN LOS NOMBRES\n");
    }
    //DEVUELVE UN 1 NO HAY ERROR Y UN 0 SI HAY ERROR
    return correcto;
}

//IMPRIME EL CONTENIDO DEL FICHERO
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
    
    if(existeFichero(directorio, nombre)==1){
        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            //IMPRIME LOS DATOS GUARDADOS EN EL BLOQUE 
            if(inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[i]!=0xFFFF){
                printf("%s\n",memdatos[inodos->blq_inodos[directorio[BuscaFich(directorio,NULL, nombre)].dir_inodo].i_nbloque[i]-4].dato);
            }
        }
    }else{
        printf("EL FICHERO NO EXITE\n");
    }
}

//COMPRUEBA SI EXITE EL FICHERO O NO, SI NO EXITE DEVUELVE UN 0
int existeFichero(EXT_ENTRADA_DIR *directorio, char* nombre){
    int error = 0;
    for(int i=1; i<MAX_FICHEROS;i++){
        if(strcmp(directorio[i].dir_nfich,nombre)==0){
            error=1;
        }
    }
    return error;
}

//DEVUELVE LA POSICION EN LA QUE SE ENCUENTRA EL FICHERO [0-20]
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
    int fichero = -1;
    for(int i=1; i<MAX_FICHEROS;i++){
        if(strcmp(directorio[i].dir_nfich,nombre)==0){
            fichero=i;
        }
    }
    return fichero;
}

//BORRA EL FICHERO
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
    int correcto=0;
    //COMPRUEBA SI EL FICHERO EXISTE O NO
    if(existeFichero(directorio, nombre)==1){
        //ELIMINAMOS EL INODO Y SUMAMOS 1 EN EL INODO GLOBAL
        ext_bytemaps->bmap_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo] =0;
        ext_superblock->s_free_inodes_count++;
        //ELIMINA CADA BLOQUE Y POR CADA BLOQUE ELIMINADO AUMENTA EN 1 EL BLOQUE GLOBAL
        for(int x=0; x<MAX_NUMS_BLOQUE_INODO; x++){
            if(inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[x] != 0xFFFF){
                ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[x]]=0;
                ext_superblock->s_free_blocks_count++;
            }
        }
        //ELIMINA LOS NUMEROS DE BLOQUE
        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].i_nbloque[i]=0xFFFF;
        }
        //ELIMINAMOS EL TAMAÑO DEL FICHERO Y LA DIRECCION DEL INODO, Y EL NOMBRE DEL FICHERO LO IGUALAMOS A UN STRING VACIO
        inodos->blq_inodos[directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo].size_fichero=0;
        directorio[BuscaFich(directorio, NULL,nombre)].dir_inodo=0xFFFF;
        strcpy(directorio[BuscaFich(directorio, NULL,nombre)].dir_nfich,"");
        printf("SE ELIMINO EL FICHERO %s\n", nombre);
        correcto=1;
    }else{
        printf("NO EXISTE EL FICHERO  %s\n", nombre);
        correcto=0;
    }
    //DEVUELVE UN 0 SI HAY ERROR
    return correcto;

}

//FUCNION QUE COPIA UN FICHERO A OTRO
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,
EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
    int correcto=0;
    int contador=0;
    int num_fichero;
    //SI EL FICHERO DESTINO YA EXISTE, SE ELIMINAN TODOS SUS DATOS PARA SOBRESCRIBIR LOS DATOS DEL FICHERO ORIGEN
    if(existeFichero(directorio, nombreorigen)==1){

        if(existeFichero(directorio, nombredestino)==1){
            Borrar(directorio,inodos,ext_bytemaps, ext_superblock, nombredestino, fich);
        }else{
            printf("SE CREO EL FICHERO %s\n", nombredestino);
        }

        //ASIGNAMOS LA DIRECCION DE INODO, EL NOMBRE, Y ACTUALIZAMOS EL INODO DEL BYTMAPS Y DEL INODO GLOBAL
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
        //ASIGNAMOS EL TAMAÑO
        inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].size_fichero=inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombreorigen)].dir_inodo].size_fichero;

        //ASIGNAMOS LOS BLOQUES Y LOS DATOS DEL BLOQUE, Y ACTUALIZAMOS LOS BLOQUES EN EL BYTMAPS Y EN LOS BLOQUES GLOBALES
        for(int i=0; i<MAX_NUMS_BLOQUE_INODO; i++){
            if(inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombreorigen)].dir_inodo].i_nbloque[i]!=0xFFFF){
                for(int t=0; t<MAX_BLOQUES_DATOS;t++){
                    if(ext_bytemaps->bmap_bloques[t]==0){
                        for(int y=0;y<MAX_NUMS_BLOQUE_INODO;y++){
                            if(inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].i_nbloque[y]==0xFFFF){
                                inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombredestino)].dir_inodo].i_nbloque[y]=t;
                                memcpy(memdatos[t-4].dato , memdatos[inodos->blq_inodos[directorio[BuscaFich(directorio,NULL,nombreorigen)].dir_inodo].i_nbloque[i]-4].dato, SIZE_BLOQUE);
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
        correcto=1;
    }else{
        printf("EL FICHERO ORIGEN NO EXISTE\n");
        correcto=0;
    }

    return correcto;

}

//FUNCION QUE ESCRIBE EN EL FICHERO EL SUPERBLOQUE
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
    fwrite(ext_superblock,SIZE_BLOQUE,1, fich);
}

//FUNCION QUE ESCRIBE EN EL FICHERO EL BYTEMAPS
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
    fwrite(ext_bytemaps,SIZE_BLOQUE,1, fich);
}
//FUNCION QUE ESCRIBE EN EL FICHERO EL INODO Y DIRECTORIO
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
    fwrite(inodos,SIZE_BLOQUE,1, fich);
    fwrite(directorio,SIZE_BLOQUE,1, fich);
}

//FUNCION QUE ESCRIBE EN EL FICHERO LOS DATOS
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
    fwrite(memdatos,SIZE_BLOQUE,MAX_BLOQUES_DATOS, fich);
}
