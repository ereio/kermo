# Kermo
Kernel Module Observations and Elevator Simulation

<br>Member 1: Taylor Ereio
<br>Member 2: Zach Deimer
<br>Member 3: Ricardo Noriega

p2-Ereio-Deimer-Noriega.tar Contents:
<br>README.txt
<br>report.txt
<br>Part_One
<br>Part_Two
<br>Part_Three
<br>Makefile

<br>Completed using: Linux Mint Cinnamon distro VM && Lab computer # 24
<br>To Build:
<br>./Part_One/gcc main.c
<br>./Part_Two/make
<br>./Part_Three/test_kernel/elevator/make

<br>To Clean:
<br>./Part_Two/make clean
<br>./Part_Three/test_kernel/elevator/make clean

<br>To Run Producer and Consumer:
<br><FILL HERE>

Known Bugs:
1. Deadlocks

To Do:
1. Threading
2. Report

Additional Comments:
1. We assumed that the messages wouldn't consume more than 2048 bytes for characters 
2. 


Sources and References
- http://lxr.free-electrons.com/source/kernel/time/timeconv.c?v=2.6.33#L77
- http://stackoverflow.com/questions/12264291/c-sprintf-in-linux-kernel
- http://www.cplusplus.com/reference/cstdio/snprintf/
- http://stackoverflow.com/questions/1102542/how-to-define-an-enumerated-type-enum-in-c
- http://stackoverflow.com/questions/4983010/invalid-type-argument-of-c-structs
- http://www.cplusplus.com/reference/cstring/strcpy/
- https://arvindsraj.wordpress.com/2012/10/05/adding-hello-world-system-call-to-linux/
