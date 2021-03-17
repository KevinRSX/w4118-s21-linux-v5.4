.PHONY: default
default: test check_policy test_weight

check_policy: check_policy.c
	$(CC) $(CFLAGS) -o check_policy check_policy.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

test_weight: test_weight.c
	$(CC) $(CFLAGS) -o test_weight test_weight.c

.PHONY: clean
clean:
	rm check_policy test test_weight
