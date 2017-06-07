SOURCES := $(wildcard sources/*.c)

all:
	gcc -pthread -o sleepingbarber $(SOURCES) -I headers -lm

clean:
	rm sleepingbarber
