#compile ipk-proj1.c
all:
	gcc -std=c99 ipk-proj2.c -o ipkcpd

run:
	gcc -std=c99 ipk-proj2.c -o ipk-proj2
	./ipk-proj2 -h 127.0.0.1 -p 2023 -m udp