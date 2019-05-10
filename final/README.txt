Min-Projet Messagerie TCP
Systemes d'exploitation et reseaux - EIDD 2019
Par : Abd Bari ABBASSI
 
## Exercice 1 :
# Fichier : server1.c
# Comment le lancer :
gcc -Wall server1.c -o server
./server 42000

## Exercice 2 :
# Fichier : client2.c
# Comment le lancer :
gcc -Wall client2.c -o client
./client ::1 42000

## Exercice 3 :
# Fichier : server3.c
# Comment le lancer :
gcc -Wall server3.c -o server -pthread
./server 42000

## Exercice 4 :
# Fichier : client4.c
# Comment le lancer :
gcc -Wall client4.c -o client
./client localhost 42000

## Exercice 5 :
# Fichier : server5.c
# Comment le lancer :
gcc -Wall server5.c -o server -pthread
./server 42000

## Exercice 6 :
# Fichier : server6.c
# Comment le lancer :
gcc -Wall server6.c -o server -pthread
./server 42000

## Exercice 7 :
# Fichier : server7.c
# Comment le lancer :
gcc -Wall server7.c -o server -pthread
./server 42000
# Remarques :
La boite des messages peux contenir RECV_MAX messages (RECV_MAX=10)