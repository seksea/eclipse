#include "misc.hpp"
#include "../interfaces.hpp"
#include "../menu/menu.hpp"
#include "../menu/config.hpp"

namespace Misc {
    const char* clantagFrames[] = {
        "e $",
        "ec $",
        "ecl $",
        "ecli $",
        "eclip $",
        "eclips $",
        "eclipse $",
        "eclipse $",
        "eclipse $",
        "eclips $",
        "eclip $",
        "ecli $",
        "ecl $",
        "ec $",
    };

    typedef void (*SendClantag)(const char*, const char*);
    void clantag(CUserCmd* cmd) {
        if (!CONFIGBOOL("clantag"))
            return;

        static SendClantag sendClantag = (SendClantag)Memory::patternScan("engine_client.so", "55 48 89 E5 41 55 49 89 FD 41 54 BF");

        int frame = ((int)(cmd->tickcount / 32)) % (sizeof(clantagFrames)/sizeof(clantagFrames[0]));

        sendClantag(clantagFrames[frame], "eclipse.wtf\n\n\n\n");
    }
}