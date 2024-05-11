build:
	gcc process_generator.c -o process_generator.out -lm
	gcc clk.c -o clk.out -lm
	gcc scheduler.c -o scheduler.out -lm
	gcc process.c -o process.out -lm
	gcc test_generator.c -o test_generator.out -lm
	gcc gui.c -o gui.out -Iinclude -Llib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./gui.out
