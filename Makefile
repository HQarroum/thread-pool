examples:
	$(MAKE) -C examples/

unit_tests:
	$(MAKE) -C tests/

tests:
	$(MAKE) -C tests/ test
	$(MAKE) -C examples/ test

all: unit_tests examples tests

clean:
	$(MAKE) -C examples/ clean
	$(MAKE) -C tests/ clean

fclean:
	$(MAKE) -C examples/ fclean
	$(MAKE) -C tests/ fclean

re: fclean all

.PHONY: all tests examples fclean
