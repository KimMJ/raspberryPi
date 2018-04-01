CC = g++
OBJS = server.o client.o
TARGET = server client

.SUFFIXES : .c .o

all: $(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS)

server.o = server.cpp server.h

client.o = client.cpp client.h

clean:
	rm $(OBJS) $(TARGET)
