all: comtk cserv
cserv: cserv.o
	gcc -o cserv cserv.o
cserv.o:cserv.c
	gcc -g -c cserv.c 

comtk: comAppInit.o comcalls.o
	gcc -o comtk comAppInit.o comcalls.o -ltcl -ltk  -lm 
	
comAppInit.o: comAppInit.c
	gcc -g -c -I/usr/include/tk -I/usr/include/tcl -I/usr/include/tk8.6 -I/usr/include/tcl8.6 -I/include comAppInit.c 
comcalls.o: comcalls.c
	gcc -g -c comcalls.c

clean: object_files
	rm -f *.o
object_files: comAppInit.o comcalls.o  

