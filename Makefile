.PHONY: default
default: test check_policy test_tick

check_policy: check_policy.c
	$(CC) $(CFLAGS) -o check_policy check_policy.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

test_tick: test_tick.c
	$(CC) $(CFLAGS) -o test_tick test_tick.c

.PHONY: clean
clean:
	rm -rf check_policy test test_tick
