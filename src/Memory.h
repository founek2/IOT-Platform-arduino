#ifndef Memory_H
#define Memory_H

#define apiKeyLen 33
#define realmMaxLen 50
#define serverMaxLen 70
#define prefixLen 2
#define EEPROM_LEN 256

// enum PairStatus : uint16_t;
typedef unsigned char uint8;
typedef unsigned short int uint16_t;

enum PairStatus : uint16_t
{
    PAIR_STATUS_BLANK = 0,
    PAIR_STATUS_INIT = 0b110011001110011,
    PAIR_STATUS_PAIRED = 0b001100110001100
};

class Memory
{
    char _api_key[apiKeyLen];
    char _mqtt_server[serverMaxLen];
    char _realm[realmMaxLen];
    PairStatus _pair_status = PAIR_STATUS_BLANK;

    uint8 _addr_prefix = 0;
    uint8 _addr_server = prefixLen;
    uint8 _addr_realm = prefixLen + serverMaxLen + 2;
    uint8 _addr_api_key = prefixLen + serverMaxLen + 2 + realmMaxLen;

    void _loadPairStatus();
    void _loadApiKey();
    void _loadServerAndRealm();

public:
    void setPairStatus(PairStatus status);
    void setApiKey(const char *apiKey);
    void setServerAndRealm(const char *server, const char *realm);

    PairStatus getPairStatus();
    const char *getApiKey();
    const char *getServer();
    const char *getRealm();

    void loadEEPROM();

    void init();

    /**
     * Delete saved topic from EEPROM
     */
    void clearEEPROM();

    bool isPaired();
    bool isInit();
};

#endif