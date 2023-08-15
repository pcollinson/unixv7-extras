CFLAGS=-O -s -n

# Change this to LOCAL=-DLOCAL
# to compile versions using the .h files installed in
# sys used during installation
LOCAL=

all: stty sett getty login

stty: stty.c
	cc $(CFLAGS) $(LOCAL) -o stty stty.c

sett: sett.c
	cc $(CFLAGS) $(LOCAL) -o sett sett.c

getty: getty.c
	cc $(CFLAGS) $(LOCAL) -o getty getty.c

login: login.c
	cc $(CFLAGS) $(LOCAL) -o login login.c


clean:
	rm stty sett getty login
