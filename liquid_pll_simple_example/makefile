# makefile : simple makefile for building and running a phase-locked loop

# default target
all : pll_simple_example.png

# main program target
pll_simple_example : pll_simple_example.c
	gcc -Wall -O2 -lm -lc -o $@ $<

# output data file
pll_simple_example.csv : pll_simple_example
	./pll_simple_example > pll_simple_example.csv

# output figure (.png)
pll_simple_example.png : pll_simple_example.gnuplot pll_simple_example.csv
	gnuplot -e 'set terminal png size 800,700' $< > $@

# remove generated files
clean :
	rm -f pll_simple_example
	rm -f pll_simple_example.csv
	rm -f pll_simple_example.png
