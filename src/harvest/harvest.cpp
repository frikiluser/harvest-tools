// Copyright © 2014, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen, and
// Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include <iostream>
#include <fstream>
#include "harvest/HarvestIO.h"
#include <string.h>
#include "harvest/exceptions.h"

using namespace::std;

void parseTracks(char * arg, vector<string> & tracks, bool & lca)
{
	lca = false;
	bool list = false;
	
	char * i = arg;
	
	while ( *i != 0 )
	{
		if ( *i == ':' )
		{
			if ( lca )
			{
				cerr << "ERROR: LCA must only have 2 tracks (\"" << arg << "\")." << endl;
				exit(1);
			}
			
			lca = true;
		}
		else if ( *i == ',' )
		{
			list = true;
		}
		
		if ( lca && list )
		{
			cerr << "ERROR: Cannot use ':' and ',' when specifying tracks (\"" << arg << "\")." << endl;
			exit(1);
		}
		
		i++;
	}
	
	char * token = strtok(arg, ":,");
	
	while ( token )
	{
		tracks.push_back(token);
		token = strtok(0, ":,");
	}
}

static const char * version = "1.3";

int main(int argc, char * argv[])
{
	const char * input = 0;
	const char * output = 0;
	vector<const char *> bed;
	const char * fasta = 0;
	vector<const char *> genbank;
	const char * maf = 0;
	const char * mfa = 0;
	const char * newick = 0;
	const char * vcf = 0;
	const char * xmfa = 0;
	const char * outFasta = 0;
	const char * outMfa = 0;
	const char * outMfaFiltered = 0;
	const char * outMfaFilteredPositions = 0;
	const char * outNewick = 0;
	const char * outSnp = 0;
	const char * outVcf = 0;
	vector<string> tracks;
	bool lca = false;
	bool signature = false;
	const char * outBB = 0;
	const char * outXmfa = 0;
	bool help = false;
	bool updateBranchVals = false;
	bool clearMult = false;
	bool quiet = false;
	bool midpointReroot = false;
	
	//stdout flag
	string out1("-");
	
	for ( int i = 0; i < argc; i++ )
	{
		if ( argv[i][0] == '-' )
		{
			switch ( argv[i][1] )
			{
				case '-':
					if ( strcmp(argv[i], "--version") == 0 )
					{
						cout << version << endl;
						return 0;
					}
					else if ( strcmp(argv[i], "--midpoint-reroot") == 0 )
					{
						midpointReroot = true;
					}
					else if ( strcmp(argv[i], "--internal") == 0 )
					{
						parseTracks(argv[++i], tracks, lca);
					}
					else if ( strcmp(argv[i], "--signature") == 0 )
					{
						signature = true;
						parseTracks(argv[++i], tracks, lca);
					}
					else
					{
						printf("ERROR: Unrecognized option ('%s').\n", argv[i]);
						help = true;
					}
					break;
				case 'a': maf = argv[++i]; break;
				case 'b': bed.push_back(argv[++i]); break;
				case 'B': outBB = argv[++i]; break;
				case 'f': fasta = argv[++i]; break;
				case 'F': outFasta = argv[++i]; break;
				case 'g': genbank.push_back(argv[++i]); break;
				case 'h': help = true; break;
				case 'i': input = argv[++i]; break;
			        case 'I': outMfaFiltered = argv[++i]; outMfaFilteredPositions = "reference_positions.txt"; break;
				case 'm': mfa = argv[++i]; break;
				case 'M': outMfa = argv[++i]; break;
				case 'n': newick = argv[++i]; break;
				case 'N': outNewick = argv[++i]; break;
				case 'o': output = argv[++i]; break;
				case 'q': quiet = true; break;
				case 'S': outSnp = argv[++i]; break;
				case 'u':
					//updateBranchVals = argv[++i];
					if ( strcmp(argv[++i], "0") == 0 )
					{
						clearMult = true;
					} 
					else
					{
						updateBranchVals = true;
					}
					break; 
				case 'v': vcf = argv[++i]; break;
				case 'V': outVcf = argv[++i]; break;
				case 'x': xmfa = argv[++i]; break;
				case 'X': outXmfa = argv[++i]; break;
				
				default:
					printf("ERROR: Unrecognized option ('%s').\n", argv[i]);
					help = true;
			}
		}
	}
	
	if (help || argc < 2)
	{
		cout << "harvesttools version " << version << " options:" << endl;
		cout << "   -i <Gingr input>" << endl;
		cout << "   -b <bed filter intervals>,<filter name>,\"<description>\"" << endl;
		cout << "   -B <output backbone intervals>" << endl;
		cout << "   -f <reference fasta>" << endl;
		cout << "   -F <reference fasta out>" << endl;
		cout << "   -g <reference genbank>" << endl;
		cout << "   -a <MAF alignment input>" << endl;
		cout << "   -m <multi-fasta alignment input>" << endl;
		cout << "   -M <multi-fasta alignment output (concatenated LCBs)>" << endl;
		cout << "   -I <multi-fasta alignment output (concatenated LCBs minus filtered SNPs)>" << endl;
		cout << "   -n <Newick tree input>" << endl;
		cout << "   -N <Newick tree output>" << endl;
		cout << "   --midpoint-reroot (reroot the tree at its midpoint after loading)" << endl;
		cout << "   -o <Gingr output>" << endl;
		cout << "   -S <output for multi-fasta SNPs>" << endl;
		cout << "   -u 0/1 (update the branch values to reflect genome length)" << endl;
		cout << "   -v <VCF input>" << endl;
		cout << "   -V <VCF output>" << endl;
		cout << "     --internal <track1>,<track2>,...  #only variants that differ among tracks" << endl;
		cout << "                                        listed" << endl;
		cout << "     --internal <track1>:<track2>      #only variants that differ within LCA" << endl;
		cout << "                                        clade of <track1> and <track2>" << endl;
		cout << "     --signature <track1>,<track2>,... #only signature variants of tracks listed" << endl;
		cout << "     --signature <track1>:<track2>     #only signature variants of LCA clade of" << endl;
		cout << "                                        <track1> and <track2>" << endl;
		cout << "   -x <xmfa alignment file>" << endl;
		cout << "   -X <output xmfa alignment file>" << endl;
		cout << "   -h (show this help)" << endl;
		cout << "   -q (quiet mode)" << endl;
		exit(0);
	}
	
	HarvestIO hio;
	
	if ( input )
	{
		if ( ! quiet ) cerr << "Loading " << input << "..." << endl;
		hio.loadHarvest(input);
	}
	
	if ( mfa )
	{
		try
		{
			if ( ! quiet ) cerr << "Loading " << mfa << "..." << endl;
			hio.loadMfa(mfa, vcf == 0);
		}
		catch ( const BadInputFileException & )
		{
			cerr << "   ERROR: " << mfa << " does not look like an MFA file." << endl;
			return 1;
		}
	}
	
	if ( fasta )
	{
		if ( ! quiet ) cerr << "Loading " << fasta << "..." << endl;
		hio.loadFasta(fasta);
	}
	
	if ( maf )
	{
		try
		{
			if ( ! quiet ) cerr << "Loading " << maf << "..." << endl;
			hio.loadMaf(maf, vcf == 0, fasta);
		}
		catch ( const ReferenceList::NameNotFoundException & e )
		{
			cerr << "   ERROR: Sequence \"" << e.name << "\" not found in reference." << endl;
			return 1;
		}
		catch (const LcbList::NoCoreException & e )
		{
			cerr << "   ERROR: No alignment involving all " << e.queryCount << " sequences found in " << maf << '.' << endl;
			return 1;
		}
	}
	
	bool useSeq = hio.referenceList.getReferenceCount() == 0;
	
	try
	{
		for ( int i = 0; i < genbank.size(); i++ )
		{
			if ( ! quiet ) cerr << "Loading " << genbank[i] << "..." << endl;
			hio.loadGenbank(genbank[i], useSeq);
		}
	}
	catch ( const AnnotationList::NoSequenceException & e )
	{
		cerr << "   ERROR: No sequence in Genbank file (" << e.file << ") and no other reference loaded.\n";
		return 1;
	}
	catch ( const AnnotationList::NoAccException & e )
	{
		cerr << "   ERROR: Genbank file (" << e.file << ") does not contain accession; cannot be matched to existing reference.\n";
		return 1;
	}
	catch ( const ReferenceList::AccNotFoundException & e )
	{
		cerr << "   ERROR: Could not find a loaded reference with accession \"" << e.acc << "\"\n";
		return 1;
	}
	
	if ( xmfa )
	{
		if ( ! quiet ) cerr << "Loading " << xmfa << "..." << endl;
		hio.loadXmfa(xmfa, vcf == 0);
	}
	
	if ( newick )
	{
		if ( ! quiet ) cerr << "Loading " << newick << "..." << endl;
		
		try
		{
			hio.loadNewick(newick);
		}
		catch ( const TrackList::TrackNotFoundException & e )
		{
			cerr << "ERROR: No track named \"" << e.name << "\"" << endl;
			return 1;
		}
	}
	
	if ( vcf )
	{
		if ( ! quiet ) cerr << "Loading " << vcf << "..." << endl;
		
		try
		{
			hio.loadVcf(vcf);
		}
		catch ( const VariantList::CompoundVariantException & e )
		{
			cerr << "ERROR: Indel allele does not contain flanking reference base (line " << e.line << " of " << vcf << ")\n";
			return 1;
		}
		catch ( const VariantList::ConflictingVariantException & e )
		{
			cerr << "ERROR: Alternate allele \"" << e.snpNew << "\" conflicts with previous alternate allele \"" << e.snpOld << "\" for sample \"" << e.track << "\" (line " << e.line << " of " << vcf << ")\n";
			return 1;
		}
	}
	
	for ( int i = 0; i < bed.size(); i++ )
	{
		char * arg = new char[strlen(bed[i]) + 1];
		
		strcpy(arg, bed[i]);
		
		const char * file = strtok(arg, ",");
		const char * name = strtok(0, ",");
		const char * desc = strtok(0, "");
		
		if ( name == 0 )
		{
			printf("ERROR: no filter name for bed file %s\n", file);
			return 1;
		}
		
		if ( desc == 0 )
		{
			printf("ERROR: no filter description for bed file %s\n", file);
			return 1;
		}
		
		hio.loadBed(file, name, desc);
		delete [] arg;
	}
	
	if ( midpointReroot )
	{
		hio.phylogenyTree.midpointReroot();
	}
	
	if ( updateBranchVals )
	{
		//by default will skip this

		//Parsnp needs this to be updated, since branch length estimates not on full multi-alignment
		//if updated, set a multiplier that will be applied when outputting newick file
		//For Gingr, by default will apply this value
		//only update if needs updating, might be already set to correct value
		if (hio.phylogenyTree.getMult() == 1.0)
		{
			double mult = hio.variantList.getVariantCount()/hio.lcbList.getCoreSize();
			hio.phylogenyTree.setMult(mult);
		}
	}
	else if ( clearMult )
	{
		//reset to 1.0, in case it has been previously modified
		//will overwrite previous value upon hio write
		hio.phylogenyTree.setMult(1.0);
	}
	
	if ( output )
	{
		if (!quiet) cerr << "Writing " << output << "...\n";
		hio.writeHarvest(output);
	}
	
	if ( outFasta )
	{
		if (!quiet) cerr << "Writing " << outFasta << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outFasta) != 0) 
		{
			fout.open(outFasta);
			fp = &fout;
		}
		
		hio.writeFasta(*fp);
	}
	
	if ( outMfa )
	{
		if (!quiet) cerr << "Writing " << outMfa << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outMfa) != 0) 
		{
			fout.open(outMfa);
			fp = &fout;
		}
		
		hio.writeMfa(*fp);
	}

	if ( outMfaFiltered )
	{
	  if (!quiet) cerr << "Writing " << outMfaFiltered << " and " << outMfaFilteredPositions << " ...\n";
		
		std::ostream* fp = &cout;
		std::ostream* fp2 = &cout;
		std::ofstream fout;
		std::ofstream fout2;
		
		if (out1.compare(outMfaFiltered) != 0) 
		{

			fout.open(outMfaFiltered);
			fout2.open(outMfaFilteredPositions);
			fp = &fout;
			fp2 = &fout2;
		}
		
		hio.writeFilteredMfa(*fp, *fp2);
	}

	if ( outNewick )
	{
		if (!quiet) cerr << "Writing " << outNewick << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outNewick) != 0) 
		{
			fout.open(outNewick);
			fp = &fout;
		}
		hio.writeNewick(*fp, true);
	}
	
	if ( outSnp )
	{
		if (!quiet) cerr << "Writing " << outSnp << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outSnp) != 0) 
		{
			fout.open(outSnp);
			fp = &fout;
		}
		
		hio.writeSnp(*fp, false);
	}

	if ( outBB )
	{
		if (!quiet) cerr << "Writing " << outBB << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outBB) != 0) 
		{
			fout.open(outBB);
			fp = &fout;
		}
		
		hio.writeBackbone(*fp);
	}
	
	if ( outXmfa )
	{
		if (!quiet) cerr << "Writing " << outXmfa << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outXmfa) != 0) 
		{
			fout.open(outXmfa);
			fp = &fout;
		}
		
		hio.writeXmfa(*fp);
	}

	if ( outVcf )
	{
		if ( lca && ! hio.phylogenyTree.getRoot() )
		{
			cerr << "ERROR: No tree loaded for LCA\n";
			return 1;
		}
		
		if (!quiet) cerr << "Writing " << outVcf << "...\n";
		
		std::ostream* fp = &cout;
		std::ofstream fout;
		
		if (out1.compare(outVcf) != 0) 
		{
			fout.open(outVcf);
			fp = &fout;
		}
		
		try
		{
			hio.writeVcf
			(
				*fp,
				tracks.size() > 0 && ! lca ? &tracks : 0,
				lca ? hio.phylogenyTree.getLca
				(
					hio.trackList.getTrackIndexByFile(tracks[0]),
					hio.trackList.getTrackIndexByFile(tracks[1])
				) : 0,
				true,
				signature
			);
		}
		catch ( const TrackList::TrackNotFoundException & e )
		{
			cerr << "ERROR: No track named \"" << e.name << "\"" << endl;
			return 1;
		}
	}
	
    return 0;
}

