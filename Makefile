TARGET = main
SRC_DIR = src
CC = g++
CFLAGS = -g -Wall -Wextra

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard $(SRC_DIR)/*.cpp))
HEADERS = $(wildcard $(SRC_DIR)/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(SRC_DIR)/*.o
	-rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

uninstall:
	rm -f /usr/local/bin/$(TARGET)
