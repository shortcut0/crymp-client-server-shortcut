#ifdef _WIN64
#define NET_CHANNEL_IP_ADDRESS_OFFSET 0xd0  // 64-bit
#else
#define NET_CHANNEL_IP_ADDRESS_OFFSET 0x78  // 32-bit
#endif

#include "..\SC_Server.h"

class SC_ServerUtils {

    IGameFramework* m_pGameFramework;

public:

    void Init() {
        m_pGameFramework = gEnv->pGame->GetIGameFramework();
    }

    // ===================================================
    // Parses the comment (if any) from a single line
    // TODO: make char customizable!
    std::string StripCfgComment(const std::string& line) {
        bool InString = false;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '"')
            {
                InString = !InString;
            }
            else if (line[i] == '#' && !InString)
                
            {
                return line.substr(0, i);
            }
        }
        return line;
    }

    // TODO: move to FileSystem
    bool FileExists(const std::string& file)
    {
        return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
    }

    // 'Converts' a Channel Id to the real IP-Address behind it
    std::string CryChannelToIP(int channelId)
    {

        std::string result;

        if (!m_pGameFramework)
            return result;

        if (INetChannel* pNetChannel = m_pGameFramework->GetNetChannel(channelId))
        {
            const unsigned char* rawIP = (unsigned char*)pNetChannel + NET_CHANNEL_IP_ADDRESS_OFFSET;
            result = std::format("{}.{}.{}.{}", rawIP[3], rawIP[2], rawIP[1], rawIP[0]);
        }

        return result;
    }
};