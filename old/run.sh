#!/bin/bash

# Use example: bash run.sh bnb.cpp https://susy.ic.unicamp.br:9999/mc558a/p2 tst 0 0

alias g++='g++ -ansi -pedantic -Wall -Werror -g';
g++ $1 -o x;


if [ ! -d "$3" ]; then
	mkdir $3

	for i in $(seq $4 $5); do 
		curl -k "$2/dados/arq$i.in" > "$3/arq$i.in";
		curl -k "$2/dados/arq$i.res" > "$3/arq$i.res";
	done
fi

R='\033[0;31m'
Y='\033[0;33m'
G='\033[0;32m'
B='\033[0;34m'
M='\033[0;35m'
W='\033[0;00m' # No Color

VALGRIND='valgrind -v --log-file="$3/arq$i.val"'


for i in $(seq $4 $5); do 
	printf "${G}Teste $i\n"
	time (./x < "$3/arq$i.txt" > "$3/arq$i.out" 2> "$3/arq$i.out2") 2> "$3/arq$i.time";
	printf "${B}"
	cat "$3/arq$i.out2"
	printf "${Y}"
	cat "$3/arq$i.time"
	printf "${M}"
	#cat "$3/arq$i.val"
	printf "${R}\n"
	diff "$3/arq$i.out" "$3/arq$i.res";
	#rm -f "$3/arq$i.out";
done

printf "${W}Fim da execução\n"