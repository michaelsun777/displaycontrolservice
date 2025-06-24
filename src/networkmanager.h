#ifndef _NETWORKMANAGER_H
#define _NETWORKMANAGER_H 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string.hpp>
#include "../3rd/json/include/nlohmann/json.hpp"
#include "../3rd/cpp-httplib/httplib.h"

using namespace nlohmann;
class NetworkManager
{
public:
    NetworkManager() {}
    ~NetworkManager() {}
    bool received(json &request, json &retvalue);
    bool getNetwork(std::string content, json &retvalue);
    bool getallNetwork(json &retvalue);
    bool setNetwork(json content);
};

#endif // !_NETWORKMANAGER_H 