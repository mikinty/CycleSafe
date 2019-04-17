
default:
	gcc -Wall -pthread -o lidar lidar.c -lpigpio -lrt
	
addrtest:
	gcc -Wall -pthread -o lidar lidar.c -D TEST_ADDR -lpigpio -lrt
	
hptest:
	gcc -Wall -pthread -o lidar lidar.c -D TEST_HP -lpigpio -lrt
	
hpaddrtest:
	gcc -Wall -pthread -o lidar lidar.c -D TEST_ADDR -D TEST_HP -lpigpio -lrt
