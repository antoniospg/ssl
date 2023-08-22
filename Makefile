# Name of the project
PROJ_NAME=sslang.out

# .c files
CPP_src=$(wildcard ./src/*.cpp)

# .py src files
PY_src=$(wildcard ./tooling/*.py)

# .py generate files
PY_gen=$(wildcard ./tooling/generate_*.py)

# .h files
H_src=$(wildcard ./src/*.h)

# .hpp generated files
HPP_GEN=$(wildcard ./src/gen/*.hpp)

# Object files
OBJ=$(subst .cpp,.o,$(subst src,objects,$(CPP_src)))

# Compiler and linker
CC=clang++

# Flags for compiler
CC_FLAGS=-c         \
         -W         \
         -Wall      \
         -ansi      \
         -g      \
         -pedantic \
				 -std=c++17

# Python linter
PYLINT=pylint

# Flags for pylint
PYLINT_FLAGS= -d=R,C

# Python interpreter
PYTHON= python3

# PATH of generated .hpp
GEN_PATH= ./src/gen

# Command used at clean target
RM = rm -rf

#
# Lint python code
#
pylint: $(PY_src)
	@ echo 'Linting python file: $^'
	$(PYLINT) $(PYLINT_FLAGS) $^
	@ echo 'Finished linting python file: $^'
	@ echo ' '

#
# Generate headers
#
sync: $(PY_gen)
	@ echo 'Generate .hpp files with: $^'
	$(PYTHON) $^ $(GEN_PATH)
	@ echo 'Finished generating .hpp files for: $^'
	@ echo ' '

#
# Compilation and linking
#
all: objFolder $(PROJ_NAME)

#
# Build compilation database
#
symbols: all
	@ echo 'Building compilation database'
	make clean
	bear -- make all

$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) $^ -o $@
	@ echo 'Finished building binary: $@'
	@ echo ' '

./objects/%.o: ./src/%.cpp ./src/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

./objects/main.o: ./src/main.cpp $(H_src) $(HPP_GEN)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

objFolder:
	@ mkdir -p objects

clean:
	@ $(RM) ./objects/*.o $(PROJ_NAME) *~
	@ rmdir objects

.PHONY: all clean
