#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes) {
	if (origin != NULL && destination != NULL) {
		int i = 0;
		char c;
		while ((i < nBytes) && ((c = fgetc(origin)) != EOF)) {
			fputc(c, destination);
			i++;
		}
		return i;
	}
	return -1;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE *file) {
	int n = 0;
	char c;
	if (file != NULL) {
		do {
			fread(&c, sizeof(char), 1, file);
			n++; //Contador para la cadena de caracteres
		} while (!feof(file) && c != '\0');

		//Retrocedemos el puntero desde la posicion actual (SEEK_CUR) tantas veces como hemos avanzado
		fseek(file, n * -1, SEEK_CUR);
		char* p;
		p = (char*) malloc(n * sizeof(unsigned char)); //Reserva memoria para n caracteres de tamanyo char

		//Lee la cadena de caracteres otra vez y la guarda en memoria
		int i = 0;

		do {
			fread(&c, sizeof(char), 1, file);
			p[i] = c;
			i++;
		} while (c != 0 && c != '\0');
		return p;
	}

	return NULL;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor
 * nFiles: output parameter. Used to return the number
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE *tarFile, int *nFiles) {
	if (tarFile != NULL) {
		stHeaderEntry *p = NULL;

		fread(nFiles, sizeof(int), 1, tarFile);

		p = (stHeaderEntry *) malloc(sizeof(stHeaderEntry) * (*nFiles));

		//Carga los datos de cada fichero (nombre, tamaño) y los almacena en header
		int i;
		for (i = 0; i < (*nFiles); i++) {
			p[i].name = loadstr(tarFile);
			fread(&p[i].size, sizeof(int), 1, tarFile);
		}
		return p;
	}
	return NULL;
}

/** Creates a tarball archive
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive.
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size)
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[]) {
	//Abrimos el fichero mtar para escritura (fichero destino)
	FILE *tarFile = fopen(tarName, "wb");
	if (tarFile != NULL) {
		//Reservamos memoria (con malloc()) para un array de stHeaderEntry
		//El array tendrá tantas posiciones como ficheros en el mtar

		stHeaderEntry *p; //nombre del struct(nombre y tamaño)
		p = (stHeaderEntry*) malloc(sizeof(stHeaderEntry) * (nFiles));

		fwrite(&nFiles, sizeof(int), 1, tarFile); // Escribimos en el tarFile el numero de ficheros

		int headerIndex = sizeof(int); //Para saltarnos el tamaño de la cabecera( 4 Bytes)

		FILE *inputFile; //el archivo que queremos comprimir

		int i;
		int headerSize = headerIndex + (sizeof(unsigned int) * nFiles);

		for (i = 0; i < nFiles; i++) { //calculamos el espacio que debemos dejar para stHeaderEntry
			inputFile = fopen(fileNames[i], "r");
			if (inputFile != NULL) {
				fseek(inputFile, 0, SEEK_END); //nos colocamos al final del fichero para calcular su tamaño
				p[i].name = fileNames[i]; //guardamos nombre del fichero en stHeaderEntry
				p[i].size = ftell(inputFile); //guardamos tamaño del fichero en stHeaderEntry
				headerSize += (strlen(fileNames[i]) + 1);
				fclose(inputFile); 		//cerramos el archivo de entrada actual
			}
		}

		fseek(tarFile, headerSize, SEEK_SET); //nos colocamos en la posicion de datos (dejando reservado headerSize)

		for (i = 0; i < nFiles; i++) {
			inputFile = fopen(fileNames[i], "r");
			if (inputFile != NULL) {
				fseek(inputFile, 0, SEEK_END); //nos colocamos en la posicion final del fichero para calcular su tamaño
				int nBytes = ftell(inputFile);
				rewind(inputFile); //volvemos al inicio del fichero para luego copiar su contenido con copynFile
				copynFile(inputFile, tarFile, nBytes);
				fclose(inputFile);
			}
		}

		fseek(tarFile, headerIndex, SEEK_SET); //volvemos a la posicion 4, al inicio despues del espacio reservado para el nº de ficheros


		for (i = 0; i < nFiles; i++) {
			inputFile = fopen(fileNames[i], "r");
			if (inputFile != NULL) {
				fwrite(p[i].name, strlen(fileNames[i]) + 1, 1, tarFile);
				fwrite(&p[i].size, sizeof(int), 1, tarFile); //siempre escribimos el mismo puntero, hay que ir cambiandolo de alguna forma
				fclose(inputFile);
			}
		}
		fclose(tarFile);
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the
 * tarball's data section. By using information from the
 * header --number of files and (file name, file size) pairs--, extract files
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[]) {
	FILE *tarFile = fopen(tarName, "r");

	unsigned char* c;
	int nFiles;

	stHeaderEntry *p;
	p = (stHeaderEntry *) malloc(sizeof(stHeaderEntry) * (nFiles));

	//Leemos los datos de cabecera y lo almacenamos en p
	p = readHeader(tarFile, &nFiles);

	int i;
	for (i = 0; i < nFiles; i++) {
		//Creamos nuevo fichero con mismo nombre en modo escritura, leemos el contenido y lo copiamos en el nuevo fichero
		FILE *nuevo = fopen(p[i].name, "w");
		if (nuevo != NULL) {
			c = (unsigned char*) malloc(p[i].size);
			fread(c, sizeof(unsigned char) * p[i].size, 1, tarFile); //leemos de tarFile
			fwrite(c, sizeof(unsigned char) * p[i].size, 1, nuevo);	//escribimos en el nuevo archivo
			fclose(nuevo);
		} else
			return EXIT_FAILURE;
	}
	fclose(tarFile);

	printf("\nFile successfully extracted!\n\n");

	return EXIT_SUCCESS;
}

