CC=mpicc
SRC_TEST=cannon
SRC_CONVERSION=conversion

all:
	@make conversion
	@make cannon

cannon: $(SRC_TEST:%=%.o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cannon $(SRC_TEST:%=%.o)  -lm 

conversion: 	
		$(CC) -o conversion $(SRC_CONVERSION:%=%.cpp)

%.o : %.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $*.c -o $*.o 

clean:
	/bin/rm -f $(SRC_TEST:%=%.o) cannon
	/bin/rm -f $(SRC_CONVERSION:%=%.o) conversion

