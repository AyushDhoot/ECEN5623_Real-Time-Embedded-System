test_output.txt is the output of the feasiblity test program as compiled on my desktop computer.
Windows 10
Using GCC 8.2.1 MSYS2, MinGW64
Target x86_64-w64-mingw32

virt_box_output.txt is the output of the feasiblity test program as compiled on my desktop virtualbox.
Ubuntu 18.04
Using GCC 7.3.0
Target x86_64-linux-gnu

Test program was restructured in the following ways:
* Added Examples 5 through 9
* Added a test execution wrapper function "run_tests" which handles
	* Calculating utilization and printing it
	* Printing all service WCET's and Periods
	* Running all feasiblity tests and printing results
* The output was also minorly reformatted for readability
* Added a function "test_schedule_over_lcm" to simulate the schedule policy over the LCM of the periods using RM, LLF, or EDF
* Added functions to compute the LCM: "lcm_set", "lcm_pair", "gcd"