#ifndef _TERM_HPP
#define _TERM_HPP

#include <string>

// http://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x405.html
// http://www.commentcamarche.net/faq/6458-la-commande-tput

class Term {

    public:

        // Save Cursor sequence
        static const std::string SC;
        // Reset Cursor sequence
        static const std::string RC;
        // Clear To The End sequence
        static const std::string CLR;
        // Reset sequence
        static const std::string R;

        // Background Color and Foreground Color sequences

        static const std::string BGC_RED;
        static const std::string FGC_RED;

        static const std::string BGC_GREEN;
        static const std::string FGC_GREEN;

        static const std::string BGC_YELLOW;
        static const std::string FGC_YELLOW;

        static const std::string BGC_BLUE;
        static const std::string FGC_BLUE;

        static std::string get_bash_output(const char * cmd);
};

#endif
