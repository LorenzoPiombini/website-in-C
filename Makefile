TARGET=server
TARGETlight := web_server
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c, obj/%.o, $(SRC))
OBJlight=$(patsubst src/%.c, obj/%_light.o, $(SRC))

default: $(TARGET)

light: $(TARGETlight)
	
clean:
	rm -f obj/*.o
	rm -f server

$(TARGET): $(OBJ)
	gcc -o $@ $? -lht -lcrypto -lssl -lstrOP -fpie -pie -z relro -z now -z noexecstack  -fsanitize=address

obj/%.o: src/%.c
	gcc -Wall -g3 -c $< -o $@ -Iinclude -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -fpie -fPIE -pie -fsanitize=address

$(TARGETlight): $(OBJlight)
	gcc -o $@ $? -lcrypto -lssl -fpie -pie -z relro -z now -z noexecstack

obj/%_light.o: src/%.c
	gcc -Wall -g3 -c $< -o $@ -Iinclude -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -fpie -fPIE -pie
