TARGET=server
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c, obj/%.o, $(SRC))

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f server

$(TARGET): $(OBJ)
	gcc -o $@ $? -lht -lcrypto -lssl -lstrOP -fsanitize=address -fpie -pie -z relro -z now -z noexecstack

obj/%.o: src/%.c
	gcc -Wall -g3 -c $< -o $@ -Iinclude -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -fpie -fPIE -pie -fsanitize=address
