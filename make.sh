#!/bin/sh

clean_objs() {
	for SRC in `cat FILELIST`
	do
		rm -f ${SRC%.c}
	done
	rm -f log
}

case "$1" in
	clean) clean_objs; exit 0;;
	create) ls chat_*.c > FILELIST; exit 0;;
esac

for SRC in `cat FILELIST`
do
	# gcc -Wall -c $SRC -o ${SRC%.c}.o
	gcc -Wall $SRC -o ${SRC%.c} -I${HOME}/include
done
