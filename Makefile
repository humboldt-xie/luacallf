all:
	gcc main.c  -I lua-5.4.4/src -L lua-5.4.4/src -l lua -lm -ldl
	./a.out