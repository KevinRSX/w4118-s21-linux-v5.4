test: test.c
	$(CC) $(CFLAGS) -o test test.c

clean:
	rm test