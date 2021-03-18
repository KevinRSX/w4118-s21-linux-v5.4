.PHONY: default
default: test check_policy

check_policy: check_policy.c
	$(CC) $(CFLAGS) -o check_policy check_policy.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

.PHONY: clean
clean:
	rm -rf check_policy test
