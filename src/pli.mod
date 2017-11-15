/* declaracoes de conjuntos e parametros */

/* quantidade de cenas e dias */
param n, integer, >=1;  
/* quantidade de atores */
param m, integer, >= 1; 

/* conjunto de indices de atores */
set ATORES := {1..m}; 
/* conjunto de indices de cenas */
set CENAS := {1..n};  
/* conjunto de indices de dias de gravacao */
set DIAS := {1..n}; 

set OPCOES := {1..4};

/* matriz T */
param T{a in ATORES, c in CENAS};

/* custo diário de espera */
param c{a in ATORES};

/* numero de cenas nas quais cada ator participa */
param participacao{a in ATORES} := sum{sc in CENAS} T[a,sc];

param nA := m;
param nC := n;
param nD := n;
param salario{a in ATORES} := c[a];

/* ===> variaveis: */
/* Permutação: Indica se a cena sc será gravada no dia d */
var Permutacao{sc in CENAS, d in DIAS},binary;

s.t. unicidade_de_cenas{d in DIAS}:
	sum{sc in CENAS} Permutacao[sc,d] == 1;

s.t. unicidade_de_dias{sc in CENAS}:
	sum{d in DIAS} Permutacao[sc,d] == 1;

var Schedule{o in OPCOES, a in ATORES, d in DIAS}, binary;

s.t. define_gravacao{a in ATORES, d in DIAS}:
	sum{sc in CENAS} T[a,sc] * Permutacao[sc,d] == Schedule[2,a,d];

s.t. restringe_opcoes{a in ATORES, d in DIAS}:
	sum{o in OPCOES} Schedule[o,a,d] == 1;

s.t. decrescente{a in ATORES, d in {1..nD-1}}:
	Schedule[1,a,d] >= Schedule[1,a,d+1];

s.t.  crescente {a in ATORES, d in {1..nD-1}}:
	Schedule[4,a,d] <= Schedule[4,a,d+1];




/* Primeiro dia de gravação de cada ator a in ATORES*/
var primeiro{a in ATORES},integer, >= 1, <= nD;

/* É o número de dias que precede o primeiro dia de gravação + 1*/
s.t. define_primeiro{a in ATORES}:
	sum{d in DIAS} Schedule[1,a,d] + 1 == primeiro[a];


/* Ultimo dia de gravação de cada ator a in ATORES*/
var ultimo{a in ATORES},integer, >= 1, <= nD;
/* É o número total de dias menos o número de dias após o ultimo dia */
s.t. define_ultimo{a in ATORES}:
	sum{d in DIAS} Schedule[4,a,d] == nD - ultimo[a];


var espera{a in ATORES},integer, >= 0, <= nD;
s.t. dfine_espera{a in ATORES}:
	espera[a] == sum{d in DIAS} Schedule[3,a,d];

var custo{a in ATORES},integer, >= 0;
s.t. define_custo{a in ATORES}:
	custo[a] == espera[a] * salario[a];

var custoTotal, integer, >= 0;
s.t. define_custoTotal:
	custoTotal == sum{a in ATORES} custo[a];

/* ===> funcao objetivo */
minimize custo_minimo:
	custoTotal;

/* ===> restricoes */
/* Já apresentadas acima */

/* resolve problema */
solve;
/*
printf: "Schedule";
printf{d in DIAS} "\t Dia %d", d;
printf: "\n";

for {a in ATORES} {
	printf: "Ator %d:", a;
    printf{d in DIAS} "\t %d", sum{o in OPCOES} o*Schedule[o,a,d];
    printf: "\n";
}

printf: "Permutacao";
printf{d in DIAS} "\t Dia %d", d;
printf: "\n";

for {sc in CENAS} {
	printf: "Cena %d:", sc;
    printf{d in DIAS} "\t %d", Permutacao[sc,d];
    printf: "\n";
}

printf: "\n";
printf: "Cenas: ";
printf{sc in CENAS} "\t %d", sum{d in DIAS} d*Permutacao[sc,d];
printf: "\n";

printf: "Dias: ";
printf{d in DIAS} "\t %d", sum{sc in CENAS} sc*Permutacao[sc,d];
printf: "\n";
printf: "\n";
# Imprimindo todas info geral dos atores
printf: "Atores\t custo \t salario \t espera \t primeiro";
printf "\t ultimo \t participacao\n";

for {a in ATORES} {
	printf: "Ator %d: \t%d\t%d\t%d\t%d\t", a, custo[a], salario[a], espera[a], primeiro[a];
    printf: "%d\t%d", ultimo[a], participacao[a];
    printf: "\n";
}
*/
printf: "%d", sum{d in DIAS} d*Permutacao[1,d];
printf{sc in {2..nD}} " %d", sum{d in DIAS} d*Permutacao[sc,d];
printf: "\n";

/* ===> imprime custo da solucao encontrada */
printf: "%d\n", custoTotal;


end;
