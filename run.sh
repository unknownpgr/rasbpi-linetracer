# Continue on CTRL+C
trap ' ' INT
# Build binary
make
# Run main program
./main.o
# Run initializer
./init.o