#include "strings.hpp"
#include <string>
#include "tools.hpp"

int loadStrings(std::vector<stringBlock> *tblock)
{
    if(tblock == NULL) return -1;
    /* STRING BLOCKS
   block   description
   0001    general UI strings
   0002    character creation strings, mantras (?)
   0003    wall text/scroll/book/book title strings (*)
   0004    object descriptions (*)
   0005    object "look" descriptions, object quality states
   0006    spell names
   0007    conversation partner names, starting at string 17 for conv 1
   0008    text on walls, signs
   0009    text trap messages
   000a    wall/floor description text
   0018    debugging strings (not used ingame)
   0c00    intro cutscene text
   0c01    ending cutscene text
   0c02    tyball cutscene text
   0c03    arial cuscene text (?)
   0c18    dream cutscene 1 text "arrived"
   0c19    dream cutscene 2 text "talismans"
   ...17
   0c1a-0c21  garamon cutscene texts
   +7
   0e01-0f3a  conversation strings
   313
   */

    //  strings are stored in a huffman tree, using this struct to store branch and leaf ddata
    hnode *head = NULL;

    std::ifstream ifile;
    const std::string tfile("UWDATA\\strings.pak");

    //  load string file
    ifile.open(tfile.c_str(), std::ios::binary);

    //  was file able to be openend?
    if(!ifile.is_open()) return -1; // error opening file

    //get node count
    unsigned char nodecntbuf[2];
    int nodecount = 0;
    readBin(&ifile, nodecntbuf, 2);
    nodecount = lsbSum(nodecntbuf, 2);
    //init htree count
    hnode htree[nodecount];


    //  read in all nodes
    //  note : last node is head of tree
    for(int i = 0; i < nodecount; i++)
    {
        unsigned char nodedatabuf[4];
        if(!readBin(&ifile, nodedatabuf, 4)) std::cout << "     error at " << i << std::endl;;

        hnode newnode;
        newnode.chardata = int(nodedatabuf[0]);
        newnode.parent   = int(nodedatabuf[1]);
        newnode.left     = int(nodedatabuf[2]);
        newnode.right    = int(nodedatabuf[3]);

       htree[i] = newnode;

    }

    //set huffman tree head to last node in list
    head = &htree[nodecount-1];


    unsigned char blockcntbuf[2];
    int blockcnt = 0;
    readBin(&ifile, blockcntbuf, 2);
    blockcnt = lsbSum(blockcntbuf,2);
    std::streampos endoffile = 0;

    //read in block offsets
    block blocks[blockcnt];

    for(int i = 0; i < blockcnt; i++)
    {
        block newblock;

        unsigned char blocknumbuf[2];
        unsigned char offsetbuf[4];

        readBin(&ifile, blocknumbuf, 2);
        readBin(&ifile, offsetbuf, 4);

        newblock.blocknum = lsbSum(blocknumbuf, 2);
        newblock.offset = std::streampos(lsbSum(offsetbuf, 4));

        blocks[i] = newblock;

    }

    //get end of file position
    ifile.seekg(0, ifile.end);
    endoffile = ifile.tellg();

    //for debug purposes
    std::vector<unsigned char> teststring;

    //for each block, read in string count and string relative offsets
    //read in strings
    for(int i = 0; i < blockcnt; i++)
    {
        //jump to block offset
        ifile.seekg( blocks[i].offset);

        //get string count
        unsigned char stringcntbuf[2];
        readBin(&ifile, stringcntbuf, 2);
        blocks[i].stringcount = lsbSum(stringcntbuf, 2);
        //resize block string list
        blocks[i].strings.resize(blocks[i].stringcount);
        //resize string offsets container
        blocks[i].stringoffsets.resize(blocks[i].stringcount);

        //get string relative offsets (relative to end of block header to start of string)
        for(int n = 0; n < blocks[i].stringcount; n++)
        {
            unsigned char stringoffbuf[2];
            readBin(&ifile, stringoffbuf, 2);

            blocks[i].stringoffsets[n] = std::streampos( lsbSum(stringoffbuf, 2));

        }

        //read in strings
        for(int n = 0; n < blocks[i].stringcount; n++)
        {
            //jump to string offsets (block offset + 6 bytes + relative offset)
            ifile.seekg( blocks[i].offset + std::streampos(2) + blocks[i].stringcount*2+ blocks[i].stringoffsets[n]);

            //if(i == 0 && n == 0) std::cout << "TEST STRING OFFSET = " << std::hex << blocks[i].offset + std::streampos(6) + blocks[i].stringoffsets[n] << std::dec << std::endl;

            //read in byte one at a time, popping off bits big-endian to navigate huffman tree nodes
            //if reaching a '|' character (0x7c), end of string found.  All following bits of current
            //byte are unused.

            //init current node to head
            hnode *curnode = head;
            bool stringdone = false;

            //decoding loop
            while(!stringdone)
            {
                //read in a byte
                unsigned char bbuf[1];
                readBin(&ifile, bbuf, 1);

                int val = int(bbuf[0]);

                //check each bit, big endian
                for(int k = 7; k >= 0; k--)
                {
                    //pop bit off, determine direction
                    //if bin val == 1, take a right
                    if( (val >> k) & 0x01)
                    {
                        //if(i == testblocknum && n == teststringnum) std::cout << "1";
                        curnode = &htree[curnode->right];
                    }
                    else
                    {
                        //if(i == testblocknum && n == teststringnum) std::cout << "0";
                        curnode = &htree[curnode->left];
                    }

                    //leaf found when when left and right children are 0xff
                    if(curnode->left == 0xff && curnode->right == 0xff)
                    {
                        //as long as string terminator is not found, add char
                        if(curnode->chardata != 0x7c)
                            blocks[i].strings[n].push_back( char( curnode->chardata));
                        //else string is done
                        else stringdone = true;

                        //testing
                        //if(i == testblocknum && n == teststringnum) std::cout << "\n         = " << std::hex << curnode->chardata << std::dec << "(" << char(curnode->chardata) << ")\n";

                        //set current node back to the head
                        curnode = head;
                    }
                }
            }

        }

    }

    //copy string block info into class member
    //string counter for feedback info
    int stringcounter = 0;
    tblock->resize(blockcnt);
    for(int i = 0; i < blockcnt; i++)
    {
        (*tblock)[i].id = blocks[i].blocknum;
        (*tblock)[i].strings = blocks[i].strings;
        stringcounter += int( (*tblock)[i].strings.size());
    }

    ifile.close();
    return stringcounter;
}
