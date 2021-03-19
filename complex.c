#include <stdio.h>

int fibonacci(unsigned long long n)
{
	if (n < 1)
		return -1;
	if (n == 1)
		return 0;
	if (n == 2)
		return 1;
	if (n == 3)
		return 3;

	return fibonacci(n - 1) + (n - 1);
}

int main(void)
{
	unsigned long long n = 1;
	while (1) {
		unsigned long long sum = 0;
		for (int i = 0; i < n; i++) {
			sum+=fibonacci(i);
		}
		printf("very complex number:%lld\n", sum);

		n++;
	}
}