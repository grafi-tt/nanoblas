int main() {
	char a = 0;
	__builtin_prefetch(&a, 1, 0);
	return 0;
}
