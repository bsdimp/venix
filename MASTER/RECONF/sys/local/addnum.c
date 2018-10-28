#include	<stdio.h>

main()
{
int	i = 0;
char	s1[81], s2[81];

	while (scanf("%s%s",s1,s2) != EOF)
		printf("%3d:\t%2x:\t%s\t%s\n",i++,i,s1,s2);
}
