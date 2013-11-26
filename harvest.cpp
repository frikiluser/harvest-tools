//
//  main.cpp
//  harvest
//
//  Created by Brian Ondov on 6/19/13.
//
//

#include <iostream>
#include "HarvestIO.h"

using namespace::std;

int main(int argc, const char * argv[])
{
	const char * input = 0;
	const char * output = 0;
	const char * bed = 0;
	const char * fasta = 0;
	const char * genbank = 0;
	const char * mfa = 0;
	const char * newick = 0;
	const char * vcf = 0;
	const char * xmfa = 0;
	const char * outSnp = 0;
	const char * outVcf = 0;
	bool help = false;
	bool quiet = false;
	
	for ( int i = 0; i < argc; i++ )
	{
		if ( argv[i][0] == '-' )
		{
			switch ( argv[i][1] )
			{
				case 'b': bed = argv[++i]; break;
				case 'f': fasta = argv[++i]; break;
				case 'g': genbank = argv[++i]; break;
				case 'h': help = true; break;
				case 'i': input = argv[++i]; break;
				case 'm': mfa = argv[++i]; break;
				case 'n': newick = argv[++i]; break;
				case 'o': output = argv[++i]; break;
				case 'q': quiet = true; break;
				case 'S': outSnp = argv[++i]; break;
				case 'V': outVcf = argv[++i]; break;
				case 'v': vcf = argv[++i]; break;
				case 'x': xmfa = argv[++i]; break;
			}
		}
	}
	
	if (help || argc < 2)
	{
	  cout << "harVest usage: harvest " << endl;
	  cout << "   -i <harvest input>" << endl;
	  cout << "   -b <bed filter intervals>" << endl;
	  cout << "   -f <reference fasta>" << endl;
	  cout << "   -g <reference genbank>" << endl;
	  cout << "   -n <newick tree>" << endl;
	  cout << "   -o <hvt output>" << endl;
	  cout << "   -S <output for multi-fasta SNPs>" << endl;
	  cout << "   -V <output for VCF>" << endl;
	  cout << "   -v <input VCF>" << endl;
	  cout << "   -x <xmfa alignment file>" << endl;
	  cout << "   -h (show this help)" << endl;
	  cout << "   -q (quiet mode)" << endl;
	  exit(0);
	}
	
	HarvestIO hio;
	
	if ( input )
	{
		hio.loadHarvest(input);
	}
	
	if ( mfa )
	{
		hio.loadMFA(mfa);
		hio.loadNewick(newick);
		hio.writeHarvest(output);
		return 0;
	}
	
	if ( fasta )
	{
		hio.loadFasta(fasta);
	}
	
	if ( genbank )
	{
		hio.loadGenbank(genbank);
	}
	
	if ( xmfa )
	{
		if (!quiet)
			printf("Loading %s...\n", xmfa);
		hio.loadXmfa(xmfa, vcf == 0);
	}
	
	if ( newick )
	{
		hio.loadNewick(newick);
	}
	
	if ( vcf )
	{
		hio.loadVcf(vcf);
	}
	
	if ( bed )
	{
		hio.loadBed(bed);
	}
	
	if ( output )
	{
		hio.writeHarvest(output);
	}
	
	if ( outSnp )
	{
		if (!quiet)
			printf("Writing %s...\n", outSnp);
		hio.writeSnp(outSnp);
	}

	if ( outVcf )
	{
		if (!quiet)
			printf("Writing %s...\n", outVcf);
		hio.writeVcf(outVcf);
	}
	
    return 0;
}

