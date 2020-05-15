
install:	atob btoa tarmail untarmail btoa.1
		mv atob btoa /usr/local/bin
		cp tarmail untarmail /usr/local/bin
		cp btoa.1 /usr/man/man1/btoa.1
		cp btoa.1 /usr/man/man1/tarmail.1

btoa:		btoa.c
		cc -O -s btoa.c -o btoa

atob:		atob.c
		cc -O -s atob.c -o atob

clean:		
		rm -f atob btoa *.o
