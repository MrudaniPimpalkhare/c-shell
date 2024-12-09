# Output executable name
EXEC = a.out

# Target to compile all .c files and generate the executable
all: $(EXEC)

# Compile step: compile all .c files into an executable
$(EXEC):
	gcc *.c -o $(EXEC) -w -g

# Run the executable
run: $(EXEC)
	./$(EXEC)

# Clean up generated files
clean:
	rm -f $(EXEC)
