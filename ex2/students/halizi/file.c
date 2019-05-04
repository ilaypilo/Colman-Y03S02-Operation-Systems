#include <stdio.h>
#include <stdlib.h>

#define SUM_OPERATION 1
#define EXIT_OPERATION 4

int main() {
	int operation, num1, num2;
	while (1) {
		printf("Please enter operation\n");
		scanf("%d", &operation);
		if (operation == EXIT_OPERATION) break;
		if (operation == SUM_OPERATION) {
			printf("Please enter two numbers\n");
			scanf("%d %d", &num1, &num2);
			printf("The sum is %d\n", num1 + num2);
		}
	}

	printf("Bye\n");

	return 0;
}
