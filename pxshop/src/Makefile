pxshop_debug:
	clang -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o pxshop.o pxshop.c
	clang -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o bmptools.o bmptools.c
	clang -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -o ../pxshop_debug pxshop.o bmptools.o -lm -lncursesw

pxshop:
	clang -c -o pxshop.o pxshop.c
	clang -c -o bmptools.o bmptools.c
	clang -o ../pxshop pxshop.o bmptools.o -lm -lncursesw
