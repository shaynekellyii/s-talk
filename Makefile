CFLAGS=-Wall 
LDFLAGS=-pthread
CC=gcc
OBJECTS=main.o list.o
TARGET=s-talk

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

include depends

depends:
	$(CC) -MM $(OBJECTS:.o=.c) > depends

clean:
	rm ./$(TARGET) *.o
