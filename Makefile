all:
	gcc Server.c -o server

build_test:
	gcc Server.c -o t0
	rm t0
