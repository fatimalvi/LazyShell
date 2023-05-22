CC=gcc
TARGET=main

output:
	gcc -o main *.c -lreadline

clean:
	rm $(TARGET)

run: 
	./main

rebuild:
	gcc -o main *.c -lreadline

build:
	gcc -o main *.c -lreadline