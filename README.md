# Local-Register-Allocation
<p>
This project was done for Rutgers CS 415 project 1 of Spring semester, 2021.
</p>
<h4>Description:</h4>
<p>	This project takes an input file containing a sequence of ILOC operations,
	along with k a number of registers for the assumed allocator that will use
	the output modified ILOC file by this project, and the algorithm that will
	handle the register allocation. The desired difference between the original
	file with sequence of ILOC operations and the modified file, is that the
	output file should use no more than k registers as specified by the user.
	This project implements 3 different local register allocation algorithms,
	they are as follow
</p>
		<br> - Bottom-down allocator (designated as b)
		<br> - Simple top-down allocator (designated as s) (no MAX_LIVE)
		<br> - Top-down allocator (designated as t)
	
	
