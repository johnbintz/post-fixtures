.PHONY : test-coverage

test-coverage :
	phpunit --coverage-html coverage --syntax-check test
