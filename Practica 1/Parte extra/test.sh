#!/bin/bash
if test -e mytar -a -x mytar; then
	echo "El archivo mytar existe y es ejecutable."

	if test -e tmp; then
		echo "Directorio tmp encontrado. Borrando..."
		rm -r tmp
	fi
	
	echo "Creando un nuevo directorio tmp..."
	mkdir tmp
	cd ./tmp
	
	echo "Creando archivo 1 (file1.txt)..."
	echo "Hello World!" > file1.txt
	echo "Creando archivo 2 (file2.txt)..."
	head -n 10 /etc/passwd > file2.txt
	echo "Creando archivo 3 (fich3.dat)..."
	head -c 8 /dev/random > file3.dat

	echo "Comprimiendo archivos en filetar.mtar..."
	./../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat
	
	mkdir out
	cp filetar.mtar ./out/
	cd ./out

	./../../mytar -x -f filetar.mtar
	echo "Extrayendo el contenido de filetar.mtar en ./out/ directory..."	
	cd ..

	echo "Comprobando las diferencias entre file1.txt anterior y el file1.txt descomprimido..."
	DIFF=$(diff file1.txt ./out/file1.txt) 
	if [ "$DIFF" != "" ] 
	then
		echo diff -q file1.txt ./out/file1.txt
		exit 1
	else
		echo "LOS ARCHIVOS SON IGUALES!"
		echo "Comprobando las diferencias entre file2.txt anterior y el file2.txt descomprimido..."
	  	DIFF=$(diff file2.txt ./out/file2.txt) 
		if [ "$DIFF" != "" ]
		then
		    echo iff -q file2.txt ./out/file2.txt
		    exit 1
		else	
			echo "LOS ARCHIVOS SON IGUALES!"
			echo "Comprobando las diferencias entre file3.dat anterior y el file3.dat descomprimido..."
			DIFF=$(diff file3.dat ./out/file3.dat) 
			if [ "$DIFF" != "" ] 
			then
				echo diff -q file3.dat ./out/file3.dat
				exit 1
			else
				echo "LOS ARCHIVOS SON IGUALES!"
				cd ../..
				echo "CORRECTO!!"
				exit 0
			fi
		fi
	fi		
else
	echo "El archivo mytar no existe."
	exit 1
fi
