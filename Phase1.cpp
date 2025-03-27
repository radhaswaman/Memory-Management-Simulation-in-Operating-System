#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class OS
{
private:
    char M[100][4]; // Memory (100 blocks of 4 words)
    char IR[4];     // Instruction Register
    char R[4];      // General Purpose Register
    int SI;         // Supervisor Interrupt
    int IC;         // Instruction Counter
    bool C;         // Condition Flag
    int blockCount; // Keeps track of current block being filled
    int instructionCount; // Tracks total number of instructions loaded

public:
    void init();
    void load();
    void display();
    void startExecution();
    void executeProgram();
    int findAddress();
    void MOS();
    void read();
    void write();
    void terminate();
    ifstream readFile;
    ofstream writeFile;
    string line;
};

void OS::display()
{
    for (int i = 0; i < 100; i++)
    {
        printf("%2d", i);
        for (int j = 0; j < 4; j++)
        {
            printf("%7c", M[i][j]);
        }
        printf("\n");
    }
}

void OS::init()
{
    // Initialize memory to empty ('-')
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M[i][j] = '-';
        }
    }

    blockCount = 0;  // Start at block 0
    instructionCount = 0;  // Total instructions loaded
    IC = 0; // Instruction Counter starts at 0
}

int OS::findAddress()
{
    int address = (IR[2] - '0') * 10 + (IR[3] - '0');  
    return address;
}

void OS::read()
{
    getline(readFile, line);
    cout << line << endl;

    IR[3] = '0'; // Assuming address offset is in IR[3]
    int address = findAddress();

    int row = address;
    int col = 0;
    for (int i = 0; i < line.length(); i++)
    {
        if (i >= 40)
        {
            break;
        }
        if (col > 3)
        {
            col = 0;
            row++;
        }
        M[row][col] = line.at(i);
        col++;
    }
}

void OS::write()
{
    int address = findAddress();
    int end = address + 10;

    for (int i = address; i < end; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (M[i][j] != '-')
            {
                writeFile << M[i][j];
            }
        }
    }
    writeFile << "\n";
}

void OS::terminate()
{
    writeFile << "\n\n";
}

void OS::MOS()
{
    if (SI == 1)
    {
        read();
    }
    else if (SI == 2)
    {
        write();
    }
    else if (SI == 3)
    {
        terminate();
    }
}

void OS::executeProgram()
{
    SI = 3; // Default to halt
    C = false;

    while (IC < instructionCount)
    {
        // Fetch the instruction into IR
        for (int i = 0; i < 4; i++)
        {
            IR[i] = M[IC][i];
        }
        IC++;
        int address = findAddress();

        // Decode and execute instruction
        if (IR[0] == 'G' && IR[1] == 'D')
        {
            SI = 1; // Read
            MOS();
        }
        else if (IR[0] == 'P' && IR[1] == 'D')
        {
            SI = 2; // Write
            MOS();
        }
        else if (IR[0] == 'H') // Halt
        {
            SI = 3;
            MOS();
            break;
        }
        else if (IR[0] == 'L' && IR[1] == 'R')
        {
            for (int i = 0; i < 4; i++)
            {
                R[i] = M[address][i]; // Load register from memory
            }
        }
        else if (IR[0] == 'S' && IR[1] == 'R')
        {
            for (int i = 0; i < 4; i++)
            {
                M[address][i] = R[i]; // Store register to memory
            }
        }
        else if (IR[0] == 'C' && IR[1] == 'R')
        {
            int count = 0;
            for (int i = 0; i < 4; i++)
            {
                if (M[address][i] == R[i])
                {
                    count++;
                }
            }
            C = (count == 4); // Set condition flag
        }
        else if (IR[0] == 'B' && IR[1] == 'T')
        {
            if (C)
            {
                IC = address; // Branch to address if condition is true
                C = false;
            }
        }
        else
        {
            cout << "Invalid Job" << endl;
            return;
        }
    }
}

void OS::startExecution()
{
    IC = 0;  // Start at instruction 0
    executeProgram();
}

void OS::load()
{
    init(); // Initialize memory and variables
    int row = 0; // Start at memory row 0

    do
    {
        getline(readFile, line);
        string opcode = line.substr(0, 4); // Read first 4 chars as opcode

        if (opcode == "$AMJ")
        {
            continue;  // Skip the AMJ line, initialization is already done
        }
        else if (opcode == "$END")
        {
            blockCount = 0;  // Reset blockCount when job ends
            continue;
        }
        else if (opcode == "$DTA")
        {
            startExecution(); // Start execution once $DTA is encountered
            continue;
        }
        else
        {
            // Load instructions into memory
            int col = 0; // Column index in a row
            for (int i = 0; i < line.length(); i++)
            {
                if (col > 3)  // Each row in memory can store 4 characters
                {
                    col = 0; // Move to next row
                    row++;
                }

                M[row][col] = line.at(i);  // Store instruction in memory

                col++;

                // After loading 10 instructions, move to the next block
                if ((row + 1) % 10 == 0 && col == 4)
                {
                    row = (blockCount + 1) * 10;
                    blockCount++;
                }
            }
            instructionCount = row + 1; // Update the instruction count
        }
    } while (!readFile.eof());
}

int main()
{
    OS os;
    os.readFile.open("input.txt");
    os.writeFile.open("output.txt", ios::app);
    os.load();  // Load instructions into memory
    os.display(); // Display the memory contents
    os.readFile.close();
    os.writeFile.close();

    return 0;
}

