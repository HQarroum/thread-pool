CXX ?= g++

APP_NAME = example

OUTPUT_FILE = example_output.txt

SRC = main.cpp

CFLAGS = -std=c++11 -I../../includes -W -Wall -Werror

OBJ = $(SRC:.c=.o)

assert = $(if $(filter whatever,${CROSS_COMPILE}),$(if ${VARIABLE},,$(error Urk! Variable problem)))

%.o: %.c
	$(CXX) -o $@ -c $<

all: $(OBJ)
	$(CXX) $(CFLAGS) -o $(APP_NAME) $^

re: fclean all

clean:
	$(shell find . -name '*~' -exec rm -r {} \; -o -name '*.o' -exec rm -r {} \;)

fclean: clean
	rm -f $(APP_NAME) $(OUTPUT_FILE)

run: all
	$(shell ./$(APP_NAME) > $(OUTPUT_FILE))

test: run

.PHONY: clean fclean re run
