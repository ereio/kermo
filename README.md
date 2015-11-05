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
<br>./Part_One/gcc -o part1.x part1.c
<br>./Part_Two/make
<br>./Part_Three/test_kernel/elevator/make

<br>To Clean:
<br>./Part_Two/make clean
<br>./Part_Three/test_kernel/elevator/make clean

To Run Parts:
- Part One (in Part_One):
	strace -o log part1.x
- Part Two (in Part_Two):
	insmod x_time
	cat /proc/timed
- Part Three
	make
	navigate to src folder
	sudo insmod elevator.ko
	./../elevator_driver/consumer.x --start
	./../elevator_driver/producer.x 
	./../elevator_driver/consumer.x --stop

Known Bugs:
1. Deadlocks
2. Can add people to the list but elevator not detecting the request (deadlocking at request).
3. Not properly printing out status after adding things to the list.
4. Threads aren't stopping properly upon exiting the module.
5. Elevator is not moving.

Unfinished Portions:
1. Adding people to list, all other logic is present.

Additional Comments:
1. We assumed that the messages wouldn't consume more than 2048 bytes for characters 
