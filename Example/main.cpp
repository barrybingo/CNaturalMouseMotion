#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include "NaturalMouseMotion.h"

// credit: https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c/868894#868894
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

int main(int argc, char **argv){
    InputParser input(argc, argv);

    bool error = false;

    if (!input.cmdOptionExists("-x") || !input.cmdOptionExists("-y"))
    {
        std::cout << "Error: Missing -x and/or -y\n\n";
        error = true;
    }
    else
    {
        int x = abs(atoi(input.getCmdOption("-x").c_str()));
        int y = abs(atoi(input.getCmdOption("-y").c_str()));

        NaturalMouseMotion::MotionNature nature;

        if (input.cmdOptionExists("-g") || input.cmdOptionExists("-granny"))
        {
            std::cout << "Granny => " << x << "," << y << std::endl;
            nature = NaturalMouseMotion::DefaultNature::NewGrannyNature();
        }
        else if (input.cmdOptionExists("-a") || input.cmdOptionExists("-average"))
        {
            std::cout << "Average => " << x << "," << y << std::endl;
            nature = NaturalMouseMotion::DefaultNature::NewAverageComputerUserNature();
        }
        else if (input.cmdOptionExists("-r") || input.cmdOptionExists("-robot"))
        {
            int speed = abs(atoi(input.getCmdOption("-s").c_str()));
            if (speed == 0)
            {
                speed = abs(atoi(input.getCmdOption("-speed").c_str()));
            }

            if (speed == 0)
            {
                std::cout << "Error: Robot requires a speed value greater than zero\n";
                error = true;
            }
            else
            {
                std::cout << "Robot(" << speed << ") => " << x << "," << y << std::endl;
                nature = NaturalMouseMotion::DefaultNature::NewRobotNature(speed);
            }
        }
        else if (input.cmdOptionExists("-f") || input.cmdOptionExists("-fastGamer"))
        {
            std::cout << "FastGamer => " << x << "," << y << std::endl;
            nature = NaturalMouseMotion::DefaultNature::NewFastGamerNature();
        }
        else
        {
            std::cout << "Error: Unknown nature\n";
            error = true;
        }

        if (!error)
        {
            if (input.cmdOptionExists("-d") || input.cmdOptionExists("-debug"))
            {
                nature.debug_printer = NaturalMouseMotion::LoggerPrinterFunc{NaturalMouseMotion::DefaultProvider::DefaultPrinter()};
            }
            if (input.cmdOptionExists("-i") || input.cmdOptionExists("-info"))
            {
                nature.info_printer = NaturalMouseMotion::LoggerPrinterFunc{NaturalMouseMotion::DefaultProvider::DefaultPrinter()};
            }

            NaturalMouseMotion::Move(nature, x, y);
        }

    }

    if(error || input.cmdOptionExists("-h")){
        std::cout << "Usage " << argv[0] << " [Options] [Nature] -x -y\n"
                  << "Options:\n"
                  << "\t[-i]nfo \t-- Print info messages.\n"
                  << "\t[-d]ebug\t-- Print debug messages.\n"
                  << "Nature:\n"
                  << "\t[-g]ranny         -- Low speed, variating flow, lots of noise in movement.\n"
                  << "\t[-a]verage        -- Medium noise, medium speed, medium noise and deviation.\n"
                  << "\t[-r]obot [-s]peed -- Custom speed, constant movement, no mistakes, no overshoots.\n"
                  << "\t[-f]astGamer      -- Quick movement, low noise, some deviation, lots of overshoots.\n"
                  << "\n"
                  << "Example:\n"
                  << "\t" << argv[0] << " -robot -speed 100 -x 500 -y 500\n"
                  << "\t" << argv[0] << " -f -x 500 -y 500\n";
    }
    return 0;
}
