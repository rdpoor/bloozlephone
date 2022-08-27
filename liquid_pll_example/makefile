# makefile : simple makefile for building and running a phase-locked loop

# default target
all : pll_example.png

# main program target
pll_example : pll_example.c
	gcc -o $@ $< -Wall -O2 -lc -lm

# output data file
pll_example.dat : pll_example
	./pll_example > pll_example.dat

# output figure (.png)
pll_example.png : pll_example.gnuplot pll_example.dat
	gnuplot -e 'set terminal png size 800,700' $< > $@

# remove generated files
clean :
	rm -f pll_example
	rm -f pll_example.dat
	rm -f pll_example.png

