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
<ul>
	<li>Bottom-down allocator (designated as b)</li>
	<li>Simple top-down allocator (designated as s) (no MAX_LIVE)</li>
	<li>Top-down allocator (designated as t)</li>
</ul>
<h4>Input:</h4>
<p>	In command prompt type k a f</p>
<p>	Where k is the number of local registers to be used on target machine</p>
<p>	a is the allocation algorithm to be used on the given file</p>
<p>	f is the original file containing sequence of ILOC operation</p>
<p>	ex: 5 s file.i</p>

<h4>Output:</h4>
<p>	The modified file will be written to stdout</p>
