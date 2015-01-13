
braces: json
	cc -g -Wall src/braces.c -Isrc/ build/json.o -o build/braces

json:
	mkdir -p build
	cc -g -Wall -c src/json.c -o build/json.o

test: json
	cc -g -Wall test/json.c -Isrc/ -Itest/ build/json.o -o test/json
