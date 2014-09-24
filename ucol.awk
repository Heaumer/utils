#!/usr/bin/awk -f

BEGIN {
	if (n == 0)
		n = 1
	ARGV[ARGC] = ARGV[ARGC-1]
	ARGC++
}

FNR == 1 {
	firstread = firstread == 0 ? 1 : 0
}

firstread {
	for (i = 1; i <= NF; i++)
		if (length($i) > sizefield[i])
			sizefield[i] = length($i)
}

!firstread {
	for (i = 1; i <= NF; i++) {
		printf("%s", $i)
		for (j = 0; j < sizefield[i]-length($i)+n; j++)
			printf(" ")
	}
	printf("\n")
}
