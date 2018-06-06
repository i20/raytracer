#include <string>

#include "../headers/Term.hpp"

using namespace std;

/*static*/ const string Term::SC = Term::get_bash_output("tput sc");
/*static*/ const string Term::RC = Term::get_bash_output("tput rc");
/*static*/ const string Term::CLR = Term::get_bash_output("tput el");
/*static*/ const string Term::R = Term::get_bash_output("tput sgr0");

/*static*/ const string Term::BGC_RED = Term::get_bash_output("tput setab 1");
/*static*/ const string Term::FGC_RED = Term::get_bash_output("tput setaf 1");

/*static*/ const string Term::BGC_GREEN = Term::get_bash_output("tput setab 2");
/*static*/ const string Term::FGC_GREEN = Term::get_bash_output("tput setaf 2");

/*static*/ const string Term::BGC_YELLOW = Term::get_bash_output("tput setab 3");
/*static*/ const string Term::FGC_YELLOW = Term::get_bash_output("tput setaf 3");

/*static*/ const string Term::BGC_BLUE = Term::get_bash_output("tput setab 4");
/*static*/ const string Term::FGC_BLUE = Term::get_bash_output("tput setaf 4");

// http://stackoverflow.com/questions/7702793/how-to-redirect-output-from-reading-a-bash-script-in-c
// http://stackoverflow.com/questions/32039852/returning-output-from-bash-script-to-calling-c-function
// http://www.gnu.org/software/libc/manual/html_node/Pipe-to-a-Subprocess.html
/*static*/ string Term::get_bash_output(const char * cmd) {

    FILE * pipe = popen(cmd, "r");
    char c;
    string s;

    while ( (c = fgetc(pipe)) != EOF )
        s += c;

    pclose(pipe);

    return s;
}
