#ifndef _TERM_HPP
#define _TERM_HPP

#include <string>

using namespace std;

// http://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x405.html
// http://www.commentcamarche.net/faq/6458-la-commande-tput

class Term {

    public:

        // Save Cursor sequence
        static const string SC;
        // Reset Cursor sequence
        static const string RC;
        // Clear To The End sequence
        static const string CLR;
        // Reset sequence
        static const string R;

        // Background Color and Foreground Color sequences

        static const string BGC_RED;
        static const string FGC_RED;

        static const string BGC_GREEN;
        static const string FGC_GREEN;

        static const string BGC_YELLOW;
        static const string FGC_YELLOW;

        static const string BGC_BLUE;
        static const string FGC_BLUE;

        static string get_bash_output(const char * cmd);
};

#endif
