mywish: myAppInit.o hc11calls.o
	gcc -o mywish myAppInit.o hc11calls.o -ltk -ltcl -lm
myAppInit.o: myAppInit.c
	gcc -O0 -g -c -I/usr/include/tk -I/usr/include/tcl -I/usr/include/tk8.6 -I/usr/include/tcl8.6 myAppInit.c
hc11calls.o: hc11calls.c
	gcc -O0 -g -c hc11calls.c
clean: object_files
	rm -f *.o
object_files: myAppInit.o hc11calls.o  

