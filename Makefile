all:
	gcc -fPIC -shared -o libpicojit.so picojit.c
