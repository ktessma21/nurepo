build:
	rm -f ./a.out
	gcc -std=gnu11 -g -Wall main.c hash.c repository.c utl.c -lcrypto -lm -Wno-unused-variable -Wno-unused-function

nurepo:
	./a.out

valgrind:
	rm -f ./a.out
	gcc -std=gnu11 -g -Wall main.c hash.c -lm -Wno-unused-variable -Wno-unused-function
	valgrind --tool=memcheck --leak-check=full ./a.out


# ofiles:
# 	rm -f *.o
# 	gcc -std=c11 -g -c -Wall scanner.c
# 	gcc -std=c11 -g -c -Wall tokenqueue.c
