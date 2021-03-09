.PHONY: clean
default: test fake_test

fake_test: fake_test.c
	$(CC) $(CFLAGS) -o fake_test fake_test.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

.PHONY: clean
clean:
	rm test fake_test
