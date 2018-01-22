all:
	gcc Server.c -o server

build_test:
	gcc Server.c -o t0 > Server_Build_Test_results.txt
	rm t0 >> Server_Build_Test_results.txt
