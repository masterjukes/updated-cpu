# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -O2 -g

# Libraries
LIBS = -lSDL2 -lSDL2_ttf

# Source Files
SRC = cpu.c
OBJ = $(SRC:.c=.o)

# Output Program
TARGET = CPUPROGRAM

# Build Rule
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

# Compile Individual C Files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Build Files
clean:
	rm -f $(OBJ) $(TARGET)

# Run the Program
run: $(TARGET)
	./$(TARGET)
