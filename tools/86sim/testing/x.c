int a=123;
int b;
extern char *etext, *edata, *end;

main()
{
	int c;
	printf("CS: %x DS: %x ES: %x SS: %x\n", getcs(), getds(), getes(), getss());
	printf("&data = %x &bss = %x &stack = %x\n", &a, &b, &c);
	printf("etext = %x edata = %x end = %x\n", &etext, &edata, &end);
}
