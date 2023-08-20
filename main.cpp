//*****************************************************************************
//
//  main.cpp
//  CSCI 463 Assignment 4
//
//  Created by Daniel Widing (z1838064)
//
//*****************************************************************************
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "getopt.h" //Windows compat
#include "hex.h"
#include "memory.h"
#include "rv32i.h"

using namespace std;

/**
 * Print a usage message and abort the program.
 **/
static void usage()
{
	cerr << "Usage: [-m hex-mem-size] infile" << endl;
	cerr << "    -m specify memory size (default = 0x430)" << endl;
	exit(1);
}

/**
 * Read a file of RV32I instructions and execute them.
 **/
int main(int argc, char** argv)
{
	uint32_t memory_limit = 0x430;		// default memory size = 1072
	int opt;

	while ((opt = getopt(argc, argv, "m:")) != -1)
	{
		switch (opt)
		{
		case 'm':
			memory_limit = std::stoul(optarg, nullptr, 16);
			break;
		default: /* '?' */
			usage();
		}
	}

	if (optind >= argc)
		usage();	// missing filename

	memory mem(memory_limit);

	if (!mem.load_file(argv[optind]))
		usage();

	rv32i sim(&mem);
	sim.disasm();
	mem.dump();

	return 0;
}
