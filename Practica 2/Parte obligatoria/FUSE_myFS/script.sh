#!/bin/bash

#a) Copie dos ficheros de texto que ocupen más de un bloque (por ejemplo fuselib.c y myFS.h)
#   a nuestro SF y a un directorio temporal, por ejemplo ./temp 

rm -R -f temp
mkdir temp

MPOINT="./mount-point"
TEMP="./temp"


echo "Copiando fuseLib.c"
cp ./src/fuseLib.c $MPOINT/
cp ./src/fuseLib.c $TEMP/
read -p "Press enter..."

echo "Copiando myFS.h"
cp ./src/myFS.h $MPOINT/
cp ./src/myFS.h $TEMP/
read -p "Pulsa enter..."

ls $MPOINT -la
ls $TEMP -la
read -p "Pulsa enter..."

#b) Audite el disco y haga un diff entre los ficheros originales y los copiados en el SF.
#   Trunque el primer fichero (man truncae) en copiasTemporales y en nuestro SF de
#   manera que ocupe un bloque de datos menos.

echo "Auditando virtual-disk..."
./my-fsck virtual-disk
echo "ok!"
echo "Comprobando diferencias entre fuseLib.c y su copia..."
#DIFF=$(diff ./src/fuseLib.c $MPOINT/fuseLib.c)
if [ diff ./src/fuseLib.c $MPOINT/fuseLib.c != "" ]
then 
	echo "Los archivos NO son iguales!!"
	#echo diff -q ./src/fuseLib.c $MPOINT/fuseLib.c
	exit 1
else
	echo "LOS ARCHIVOS SON IGUALES!!"
fi

echo "Comprobando diferencias entre myFS.h y su copia..."
#DIFF=$(diff ./src/myFS.h $MPOINT/myFS.h)
if [ diff ./src/myFS.h $MPOINT/myFS.h != "" ] 
then
	echo "Los archivos NO son iguales!!"		
	#echo diff -q ./src/myFS.h $MPOINT/myFS.h
	exit 1
else
	echo "LOS ARCHIVOS SON IGUALES!!"
	echo "CORRECTO!!!!!"
fi

echo "Truncando archivos..."
truncate -s -1 -o $TEMP/fuseLib.c
truncate -s -1 -o $MPOINT/fuseLib.c
read -p "Pulsa enter..."
ls $MPOINT -la
ls $TEMP -la
read -p "Pulsa enter..."


#c) Audite el disco y haga un diff entre el fichero original y el truncado.


echo "Auditando virutal-disk"
./my-fsck virtual-disk
echo "ok!"
echo "Comprobando diferencias entre fuseLib.c y su copia..."
#DIFF=$(diff ./src/fuseLib.c $MPOINT/fuseLib.c)
if [ diff ./src/fuseLib.c $MPOINT/fuseLib.c != "" ] 
then
	echo "Los archivos NO son iguales!!"
	#echo diff -q ./src/fuseLib.c $MPOINT/fuseLib.c			
	exit 1
else
	echo "LOS ARCHIVOS SON IGUALES!!"
	echo "CORRECTO!!!!!"
fi


#d) Copie un tercer fichero de texto a nuestro SF.


echo "Copiando fuseLib.h"
cp ./src/fuseLib.h $MPOINT/
read -p "Pulsa enter..."
ls $MPOINT -la
read -p "Pulsa enter..."


#e) Audite el disco y haga un diff entre el fichero original y el copiado en el SF.


echo "Auditando virtual-disk"
./my-fsck virtual-disk
echo "ok!"
echo "Comprobando las diferencias entre fuseLib.h y su copia..."
#DIFF=$(diff ./src/fuseLib.h $MPOINT/fuseLib.h)
if [ diff ./src/fuseLib.h $MPOINT/fuseLib.h != "" ] 
then
	echo "Los archivos NO son iguales!!"				
	#echo diff -q ./src/fuseLib.h $MPOINT/fuseLib.h
	exit 1
else
	echo "LOS ARCHIVOS SON IGUALES!!"
	echo "CORRECTO!!!!!"
fi


#f) Trunque el segundo fichero en copiasTemporales y en nuestro SF haciendo que ocupe 
#   algún bloque de datos más.


echo "Truncando archivos..."
truncate -s +1 -o $TEMP/myFS.h
truncate -s +1 -o $MPOINT/myFS.h
read -p "Pulsa enter..."
ls $MPOINT -la
ls $TEMP -la
read -p "Pulsa enter..."


#g) Audite el disco y haga un diff entre el fichero original y el truncado.


echo "Auditando virtual-disk"
./my-fsck virtual-disk
echo "ok!"
echo "Comprobando diferencias entre myFS.h y el truncado..."
#DIFF=$(diff ./src/myFS.h $MPOINT/myFS.h)
if [ diff ./src/myFS.h $MPOINT/myFS.h != "" ] 
then
	echo "error"					
	#echo diff -q ./src/myFS.h $MPOINT/myFS.h
	exit 1
else
	echo "LOS ARCHIVOS SON IGUALES!!"
	echo "CORRECTO!!!!!"
fi
exit 0
