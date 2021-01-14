/*
 ============================================================================
 Name        : simul_ext.c
 Author      : jorge
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define SEPARADOR " "
#define MAXSHORT USHRT_MAX

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando();
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
		char *nombre, FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
		EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,
		FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio,
		EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main() {

	char comando[LONGITUD_COMANDO];
	char *orden;
	char *argumento1;
	char *argumento2;

	int i, j;
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

	fent = fopen("particion.bin", "r+b");
	fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

	memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK*) &datosfich[0],
	SIZE_BLOQUE);
	memcpy(&directorio, (EXT_ENTRADA_DIR*) &datosfich[3], SIZE_BLOQUE);
	memcpy(&ext_bytemaps, (EXT_BYTE_MAPS*) &datosfich[1], SIZE_BLOQUE);
	memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS*) &datosfich[2], SIZE_BLOQUE);
	memcpy(&memdatos, (EXT_DATOS*) &datosfich[4],
	MAX_BLOQUES_DATOS * SIZE_BLOQUE);

	for (;;) {
		do {
			setvbuf(stdout, NULL, _IONBF, 0);
			printf(">>");
			fflush(stdin);
			fgets(comando, LONGITUD_COMANDO, stdin);
			comando[strcspn(comando, "\n")] = '\0';
			orden = strtok(comando, SEPARADOR);
			argumento1 = strtok(NULL, SEPARADOR);
			argumento2 = strtok(NULL, SEPARADOR);
		} while (ComprobarComando(comando) != 0);

		if (strcmp(orden, "dir") == 0) {
			Directorio(&directorio, &ext_blq_inodos);
			continue;
		}
		if (strcmp(orden, "bytemaps") == 0) {
			Printbytemaps(&ext_bytemaps);
			continue;
		}
		if (strcmp(orden, "copy") == 0) {
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento1);
			if (entradadir == 0) {
				printf("ERROR: Fichero %s no encontrado\n", argumento1);
				continue;
			}
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento2);
			if (entradadir == 1) {
				printf("ERROR: Fichero %s ya existe\n", argumento2);
				continue;
			}
			grabardatos = Copiar(&directorio, &ext_blq_inodos, &ext_bytemaps,
					&ext_superblock, &memdatos, argumento1, argumento2, fent);
			Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
			GrabarByteMaps(&ext_bytemaps, fent);
			GrabarSuperBloque(&ext_superblock, fent);
			continue;
		}
		if (strcmp(orden, "remove") == 0) {
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento1);
			if (entradadir == 0) {
				printf("ERROR: Fichero %s no encontrado\n", argumento1);
				continue;
			}
			Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock,
					argumento1, fent);
			Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
			GrabarByteMaps(&ext_bytemaps, fent);
			GrabarSuperBloque(&ext_superblock, fent);
			continue;
		}
		if (strcmp(orden, "info") == 0) {
			LeeSuperBloque(&ext_superblock);
			continue;
		}
		if (strcmp(orden, "imprimir") == 0) {
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento1);
			if (entradadir == 0) {
				printf("ERROR: Fichero %s no encontrado\n", argumento1);
				continue;
			}
			Imprimir(&directorio, &ext_blq_inodos, &memdatos, argumento1);
			continue;
		}
		if (strcmp(orden, "rename") == 0) {
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento1);
			if (entradadir == 0) {
				printf("ERROR: Fichero %s no encontrado\n", argumento1);
				continue;
			}
			entradadir = BuscaFich(&directorio, &ext_blq_inodos, argumento2);
			if (entradadir == 1) {
				printf("ERROR: Fichero %s ya existe\n", argumento2);
				continue;
			}
			Renombrar(&directorio, &ext_blq_inodos, argumento1, argumento2);
			Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
			continue;
		}

		// Escritura de metadatos en comandos rename, remove, copy
		if (grabardatos)
			GrabarDatos(&memdatos, fent);
		grabardatos = 0;

		//Si el comando es salir se habrán escrito todos los metadatos
		//faltan los datos y cerrar
		if (strcmp(orden, "salir") == 0) {
			GrabarDatos(&memdatos, fent);
			fclose(fent);
			return 0;
		}
	}

	return 0;
}

int ComprobarComando(char *comando) {
	int num = 1;

	comando[strcspn(comando, "\n")] = '\0';
	char *orden = strtok(comando, SEPARADOR);

	if (strcmp(orden, "bytemaps") == 0) {
		num = 0;
	} else if (strcmp(orden, "copy") == 0) {
		num = 0;
	} else if (strcmp(orden, "dir") == 0) {
		num = 0;
	} else if (strcmp(orden, "info") == 0) {
		num = 0;
	} else if (strcmp(orden, "imprimir") == 0) {
		num = 0;
	} else if (strcmp(orden, "rename") == 0) {
		num = 0;
	} else if (strcmp(orden, "remove") == 0) {
		num = 0;
	} else if (strcmp(orden, "salir") == 0) {
		num = 0;
	}

	if (num == 1) {
		printf(
				"ERROR: Comando ilegal [bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
	}

	return num;
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {

	printf("Inodos: ");
	for (int i = 0; i < 24; i++) {
		printf("%u ", ext_bytemaps->bmap_inodos[i]);
	}
	printf("\n");
	printf("Bloques [0-25]: ");
	for (int i = 0; i < 25; i++) {
		printf("%u ", ext_bytemaps->bmap_bloques[i]);
	}
	printf("\n");
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
	printf("Bloque = %d\n", psup->s_block_size);
	printf("Inodos particion = %d\n", psup->s_inodes_count);
	printf("inodos libres = %d\n", psup->s_free_inodes_count);
	printf("Bloques particion = %d\n", psup->s_blocks_count);
	printf("Bloques libres = %d\n", psup->s_free_blocks_count);
	printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
	int num = 0;
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo != 0xffff) {
			if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
				num = 1;
			}
		}
	}
	return num;
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		char *nombreantiguo, char *nombrenuevo) {
	int num = 0;
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo != 0xffff) {
			if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) {
				strcpy(directorio[i].dir_nfich, nombrenuevo);
			}
		}
	}
	return 0;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo != 0xffff) {
			printf("%s  ", directorio[i].dir_nfich);
			printf("tamano:");
			printf("%d  ",
					inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
			printf("inodo:");
			printf("%d  ", directorio[i].dir_inodo);
			printf("bloques:");
			for (int j = 0; j < 7; j++) {
				if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
						!= 0xffff) {
					printf("%d ",
							inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
				}
			}
			printf("\n");
		}
	}
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
		EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,
		FILE *fich) {
	int n_inodo;
	int n_bloques = 0;
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo == 0xffff) {
			strcpy(directorio[i].dir_nfich, nombredestino);
			for (int j = 0; j < 24; j++) {
				if (ext_bytemaps->bmap_inodos[j] == 0) {
					ext_bytemaps->bmap_inodos[j] = 1;
					directorio[i].dir_inodo = j;
					n_inodo = j;
					break;
				}
			}
			break;
		}
	}
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo != 0xffff) {
			if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
				for (int j = 0; j < 7; j++) {
					if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
							!= 0xffff) {
						for (int k = 0; k < 100; k++) {
							if (ext_bytemaps->bmap_bloques[k] == 0) {
								ext_bytemaps->bmap_bloques[k] = 1;
								memdatos[k - 4] =
										memdatos[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
												- 4];
								inodos->blq_inodos[n_inodo].i_nbloque[j] = k;
								n_bloques += 1;
								break;
							}
						}
					}
				}
				inodos->blq_inodos[n_inodo].size_fichero =
						inodos->blq_inodos[directorio[i].dir_inodo].size_fichero;
			}
		}
	}
	ext_superblock->s_free_blocks_count = ext_superblock->s_free_blocks_count
			- n_bloques;
	ext_superblock->s_free_inodes_count = ext_superblock->s_free_inodes_count
			- 1;
	return 1;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
		char *nombre, FILE *fich) {
	int n_bloques = 0;

	for (int i = 1; i < 20; i++) {
		if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
			strcpy(directorio[i].dir_nfich, " ");
			inodos->blq_inodos[directorio[i].dir_inodo].size_fichero = 0;
			for (int j = 0; j < 7; j++) {
				if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
						!= 0xffff) {
					ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]] =
							0;
					inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] =
							0xffff;
					n_bloques += 1;
				}
			}
			directorio[i].dir_inodo = 0xffff;
		}
	}
	ext_superblock->s_free_blocks_count = ext_superblock->s_free_blocks_count
			+ n_bloques;
	ext_superblock->s_free_inodes_count = ext_superblock->s_free_inodes_count
			+ 1;
	return 0;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
		EXT_DATOS *memdatos, char *nombre) {
	for (int i = 1; i < 20; i++) {
		if (directorio[i].dir_inodo != 0xffff) {
			if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
				for (int j = 0; j < 7; j++) {
					if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
							!= 0xffff) {
						printf("%s\n",
								memdatos[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]
										- 4].dato);
					}
				}
			}
		}
	}
	return 0;
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
	fseek(fich, SIZE_BLOQUE, SEEK_SET);
	fwrite(&ext_bytemaps, 1, sizeof(EXT_BYTE_MAPS), fich);
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio,
		EXT_BLQ_INODOS *inodos, FILE *fich) {

	fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
	fwrite(directorio, 1, sizeof(EXT_ENTRADA_DIR), fich);

	fseek(fich, SIZE_BLOQUE * 2, SEEK_SET);
	fwrite(inodos, 1, sizeof(EXT_BLQ_INODOS), fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
	fseek(fich, 0, SEEK_SET);
	fwrite(ext_superblock, 1, sizeof(EXT_SIMPLE_SUPERBLOCK), fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
	fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);
	fwrite(memdatos, 1, sizeof(EXT_DATOS), fich);
}
