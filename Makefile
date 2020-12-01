#NAME: Jai Vardhan Fatehpuria
#EMAIL: jaivardhan.f@gmail.com
#ID: 804817305

default: 
	gcc -o lab4b -g -Wall -Wextra -lmraa -lm lab4b.c

check:	default
	chmod +x check.sh
	./check.sh

clean:
	rm -f lab4b lab4b-804817305.tar.gz

dist:
	tar -czvf lab4b-804817305.tar.gz lab4b.c Makefile check.sh README