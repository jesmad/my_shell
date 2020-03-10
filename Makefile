HEADERS = parse.h			
OBJECTS = parse.o main.o		

default: nsh				

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

nsh: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f nsh
