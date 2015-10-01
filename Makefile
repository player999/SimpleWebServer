TARGET=webserver
all: clean
	gcc -o $(TARGET) ./webserver.c

clean:
	rm -f ./$(TARGET)

