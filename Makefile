
braces: json
	cc -g -Wall src/braces.c -Isrc/ build/json.o -o build/braces

json:
	mkdir -p build
	cc -g -Wall -c src/json.c -o build/json.o

test: 
	tests/test.sh
