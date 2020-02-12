.PHONY=all

all:
	gcc controller.c -o controller
	cd module && $(MAKE)

clean:
	rm controller
	cd module && $(MAKE) clean 
