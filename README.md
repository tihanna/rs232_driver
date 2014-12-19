RS232 Driver implementation
===========================

Example Linux kernel module

Building and testing
--------------------

1. Kompajlira se sa: make
2. Ostaci od kompajliranja se brišu sa: make clean
3. Skripte za kreiranje /dev i ostalog se mogu instalirati sa: sudo make install
4. Te iste skripte se brišu sa: sudo make uninstall
5. .ko fajl se nalazi u folderu obj
6. Za učitavanje, uklanjanje ili "reload" modula mogu se koristiti, redom:
   sudo make ld
   sudo make ud
   sudo make rld

