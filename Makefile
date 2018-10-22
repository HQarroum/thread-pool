examples:
	$(MAKE) -C examples/

test:
	$(MAKE) -C examples/ test
	$(MAKE) -C tests/ test

all: tests examples

clean:
	$(MAKE) -C examples/ clean
	$(MAKE) -C tests/ clean

fclean:
	$(MAKE) -C examples/ fclean
	$(MAKE) -C tests/ fclean

re: fclean all

.PHONY: all examples tests fclean
