#include "networkmanager.h"

using namespace nlohmann;

typedef enum _IP_Type_
{
    IPV4 = 4,
    IPV6 = 6
} IPType;

int GetResultFromSystemCall(const char *command, std::string &retvalue)
{
    if (!command)
    {
        return -1;
    }
    FILE *fp = popen(command, "r");
    if (fp)
    {
        char buffer[256] = {0};
        while (fgets(buffer, sizeof(buffer), fp))
        {
            retvalue.append(buffer);
        }
        pclose(fp);
        return 0;
    }
    return -1;
}

std::string GetMacByInterfaceName(const char *interface_name)
{
    std::string macAddr;
    char szMac[256] = {0};
    struct ifreq ifreq;
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        close(sock);
        return macAddr;
    }
    strcpy(ifreq.ifr_name, interface_name);
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        close(sock);
        return macAddr;
    }
    sprintf(szMac, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
    macAddr.assign(szMac);
    close(sock);
    return macAddr;
}

static std::string getNetMask(int pos)
{
    if (pos > 31 || pos < 0)
    {
        return "";
    }
    pos = 16;
    unsigned int i = (((1 << pos) - 1) << (32 - pos));
    unsigned int one = i & 255;
    unsigned int two = (i & (255 << 8)) >> 8;
    unsigned int three = (i & (255 << 16)) >> 16;
    unsigned int four = (i & (255 << 24)) >> 24;
    return std::to_string(four) + "." + std::to_string(three) + "." + std::to_string(two) + "." + std::to_string(one);
}

static int getNetMaskNumOfBit(const char *ip)
{
    if (!ip)
    {
        return -1;
    }
    struct in_addr addr;
    inet_aton(ip, &addr);
    int v = addr.s_addr;
    int count = 0;
    while (v)
    {
        v &= (v - 1);
        count++;
    }
    return count;
}

static std::string ipMethod(const char *interface_name, IPType type)
{
    std::string devices;
    GetResultFromSystemCall("nmcli connection show | grep - | awk '{print $(NF-2)}'", devices);
    std::vector<std::string> device_list;
    std::string device;
    std::istringstream iss(devices);
    while (std::getline(iss, device))
    {
        device_list.push_back(device);
    }
    for (auto &d : device_list)
    {
        bool ok = false;
        std::string deviceContent;
        std::string cmd = "nmcli con show ";
        cmd += d;
        GetResultFromSystemCall(cmd.c_str(), deviceContent);
        std::istringstream ss(deviceContent);
        std::vector<std::string> device_info;
        std::string info;
        while (std::getline(ss, info))
        {
            device_info.push_back(info);
        }
        for (auto &i : device_info)
        {
            if (boost::starts_with(i, "connection.interface-name"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            else if (boost::starts_with(i, "GENERAL.DEVICES"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            std::string ipType = "ipv";
            ipType += std::to_string(type);
            if (ok && boost::starts_with(i, ipType + ".method"))
            {
                return i.substr(i.rfind(" ") + 1);
            }
        }
    }
    return "";
}

static std::string getGateway(const char *interface_name, IPType type)
{
    std::string devices;
    GetResultFromSystemCall("nmcli connection show | grep - | awk '{print $(NF-2)}'", devices);
    std::vector<std::string> device_list;
    std::string device;
    std::istringstream iss(devices);
    while (std::getline(iss, device))
    {
        device_list.push_back(device);
    }
    for (auto &d : device_list)
    {
        bool ok = false;
        std::string deviceContent;
        std::string cmd = "nmcli con show ";
        cmd += d;
        GetResultFromSystemCall(cmd.c_str(), deviceContent);
        std::istringstream ss(deviceContent);
        std::vector<std::string> device_info;
        std::string info;
        while (std::getline(ss, info))
        {
            device_info.push_back(info);
        }
        for (auto &i : device_info)
        {
            if (boost::starts_with(i, "connection.interface-name"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            else if (boost::starts_with(i, "GENERAL.DEVICES"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            std::string ipType = "IP";
            ipType += std::to_string(type);
            if (ok && boost::starts_with(i, ipType + ".GATEWAY"))
            {
                std::string gateway = i.substr(i.rfind(" ") + 1);
                if (gateway.find("--") != std::string::npos)
                    return "";
                else
                    return gateway;
            }
        }
    }
    return "";
}

static std::string getIpAddr(const char *interface_name, IPType type)
{
    std::string devices;
    GetResultFromSystemCall("nmcli connection show | grep - | awk '{print $(NF-2)}'", devices);
    std::vector<std::string> device_list;
    std::string device;
    std::istringstream iss(devices);
    while (std::getline(iss, device))
    {
        device_list.push_back(device);
    }
    for (auto &d : device_list)
    {
        bool ok = false;
        bool isAuto = false;
        std::string deviceContent;
        std::string cmd = "nmcli con show ";
        cmd += d;
        GetResultFromSystemCall(cmd.c_str(), deviceContent);
        std::istringstream ss(deviceContent);
        std::vector<std::string> device_info;
        std::string info;
        while (std::getline(ss, info))
        {
            device_info.push_back(info);
        }
        for (auto &i : device_info)
        {
            if (boost::starts_with(i, "connection.interface-name"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            else if (boost::starts_with(i, "GENERAL.DEVICES"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            std::string ipType1 = "ipv";
            std::string ipType2 = "IP";
            ipType1 += std::to_string(type);
            ipType2 += std::to_string(type);
            if (boost::starts_with(i, ipType1 + ".method"))
            {
                if (i.find("auto") != std::string::npos)
                    isAuto = true;
            }
            if (ok)
            {
                std::string ip;
                if (boost::starts_with(i, ipType1 + ".addresses") && !isAuto)
                {
                    ip = i.substr(i.rfind(" ") + 1);
                    if (ip == "--")
                        ip = "";
                    return ip;
                }
                if (boost::starts_with(i, ipType2 + ".ADDRESS") && isAuto)
                {
                    ip = i.substr(i.rfind(" ") + 1);
                    if (ip == "--")
                        ip = "";
                    return ip;
                }
            }
        }
    }
    return "";
}

static std::vector<std::string> getDns(const char *interface_name, IPType type)
{
    std::string devices;
    std::vector<std::string> result;
    GetResultFromSystemCall("nmcli connection show | grep - | awk '{print $(NF-2)}'", devices);
    std::vector<std::string> device_list;
    std::string device;
    std::istringstream iss(devices);
    while (std::getline(iss, device))
    {
        device_list.push_back(device);
    }
    for (auto &d : device_list)
    {
        bool ok = false;
        std::string deviceContent;
        std::string cmd = "nmcli con show ";
        cmd += d;
        GetResultFromSystemCall(cmd.c_str(), deviceContent);
        std::istringstream ss(deviceContent);
        std::vector<std::string> device_info;
        std::string info;
        while (std::getline(ss, info))
        {
            device_info.push_back(info);
        }
        for (auto &i : device_info)
        {
            if (boost::starts_with(i, "connection.interface-name"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            else if (boost::starts_with(i, "GENERAL.DEVICES"))
            {
                if (i.find(interface_name) != std::string::npos)
                {
                    ok = true;
                }
            }
            std::string ipType = "IP";
            ipType += std::to_string(type);
            if (ok && boost::starts_with(i, ipType + ".DNS[1]"))
            {
                result.push_back(i.substr(i.rfind(" ") + 1));
            }
            else if (ok && boost::starts_with(i, ipType + ".DNS[2]"))
            {
                result.push_back(i.substr(i.rfind(" ") + 1));
            }
        }
    }
    return result;
}

// class NetworkManager
// {
// public:
//     NetworkManager() {}
//     ~NetworkManager() {}
//     bool received(json &request, json &retvalue);
//     bool getNetwork(std::string content, json &retvalue);
//     bool getallNetwork(json &retvalue);
//     bool setNetwork(json content);
// };

bool NetworkManager::received(json &request, json &retvalue)
{
    try
    {
        std::string op = request["op"].get<std::string>();
        if (op == "get")
        {
            std::string content = request["content"].get<std::string>();
            return getNetwork(content, retvalue);
        }
        else if (op == "set")
        {
            json content = request["content"];
            return setNetwork(content);
        }
        else if (op == "getall")
        {
            return getallNetwork(retvalue);
        }
        else
        {
            retvalue = "unknown op";
            return false;
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
}

bool NetworkManager::getNetwork(std::string content, json &retvalue)
{
    if (content.empty())
    {
        return false;
    }
    std::string ipaddr;
    std::string ipaddr6;
    std::string netmask;
    std::string macaddr;
    std::string gateway;
    std::string gateway6;
    std::string dnsprimary;
    std::string dnssecondary;
    std::string subnetlength;
    std::string linkmode;
    bool dhcp;
    bool dhcp6;
    json retJson;
    YAML::Node yaml = YAML::LoadFile("/etc/netplan/01-network-manager-all.yaml");
    if (yaml["network"].IsDefined() && !yaml["network"]["ethernets"].IsNull() && yaml["network"]["ethernets"][content.c_str()].IsDefined())
    {
        auto net = yaml["network"]["ethernets"][content.c_str()];
        if (net["dhcp4"].IsDefined())
        {
            dhcp = net["dhcp4"].as<bool>();
        }
        if (net["dhcp6"].IsDefined())
        {
            dhcp6 = net["dhcp6"].as<bool>();
        }
        if (net["gateway4"].IsDefined())
        {
            gateway = net["gateway4"].as<std::string>();
        }
        if (net["gateway6"].IsDefined())
        {
            gateway6 = net["gateway6"].as<std::string>();
        }
        if (net["nameservers"].IsDefined() && net["nameservers"]["addresses"].IsDefined())
        {
            for (int n = 0; n < net["nameservers"]["addresses"].size(); ++n)
            {
                if (n == 0)
                    dnsprimary = net["nameservers"]["addresses"][n].as<std::string>();
                if (n == 1)
                {
                    dnssecondary = net["nameservers"]["addresses"][n].as<std::string>();
                    break;
                }
            }
        }
    }
    else
    {
        dhcp = true;
        dhcp6 = true;
        std::string method6 = ipMethod(content.c_str(), IPType::IPV6);
        if (method6 == "ignored")
        {
            dhcp6 = false;
        }
        gateway = getGateway(content.c_str(), IPType::IPV4);
        gateway6 = getGateway(content.c_str(), IPType::IPV6);
        auto dns = getDns(content.c_str(), IPType::IPV4);
        int dnsSize = dns.size();
        if (dnsSize == 1)
        {
            dnsprimary = dns[0];
        }
        else if (dnsSize == 2)
        {
            dnsprimary = dns[0];
            dnssecondary = dns[1];
        }
    }
    macaddr = GetMacByInterfaceName(content.c_str());
    std::string ip4 = getIpAddr(content.c_str(), IPType::IPV4);
    std::string ip6 = getIpAddr(content.c_str(), IPType::IPV6);
    if (!ip4.empty())
    {
        ipaddr = ip4.substr(0, ip4.find("/"));
        int pos = std::atoi(ip4.substr(ip4.find("/") + 1).c_str());
        netmask = getNetMask(pos);
    }
    if (!ip6.empty())
    {
        ipaddr6 = ip6.substr(0, ip6.find("/"));
        subnetlength = ip6.substr(ip6.find("/") + 1);
    }
    retJson["device"] = content;
    if (dhcp)
        retJson["ipv4"]["dhcp"] = true;
    else
        retJson["ipv4"]["dhcp"] = false;
    if (dhcp6)
        retJson["ipv6"]["dhcp"] = true;
    else
        retJson["ipv6"]["dhcp"] = false;
    retJson["ipv4"]["address"] = ipaddr;
    retJson["ipv4"]["netmask"] = netmask;
    retJson["ipv4"]["gateway"] = gateway;
    retJson["dns"]["primary"] = dnsprimary;
    retJson["dns"]["secondary"] = dnssecondary;
    retJson["ipv6"]["address"] = ipaddr6;
    retJson["ipv6"]["subnetlength"] = subnetlength;
    retJson["ipv6"]["gateway"] = gateway6;
    retJson["mac"] = macaddr;
    retvalue = retJson;
    return true;
}

bool NetworkManager::setNetwork(json content)
{
    YAML::Node yaml = YAML::LoadFile("/etc/netplan/01-network-manager-all.yaml");
    std::string device = content["device"].get<std::string>();
    bool dhcp = content["ipv4"]["dhcp"].get<bool>();
    std::string ipaddr = content["ipv4"]["address"].get<std::string>();
    std::string netmask = content["ipv4"]["netmask"].get<std::string>();
    std::string gateway = content["ipv4"]["gateway"].get<std::string>();
    bool dhcp6 = content["ipv6"]["dhcp"].get<bool>();
    std::string ipaddr6 = content["ipv6"]["address"].get<std::string>();
    std::string subnetlength = content["ipv6"]["subnetlength"].get<std::string>();
    std::string gateway6 = content["ipv6"]["gateway"].get<std::string>();
    std::string dnsprimary = content["dns"]["primary"].get<std::string>();
    std::string dnssecondary = content["dns"]["secondary"].get<std::string>();
    if (!device.empty())
    {
        yaml["network"]["ethernets"][device]["dhcp4"] = dhcp;
        yaml["network"]["ethernets"][device]["dhcp6"] = dhcp6;
        if (dhcp == false && !netmask.empty() && !ipaddr.empty())
        {
            int count = getNetMaskNumOfBit(netmask.c_str());
            if (count == -1)
            {
                return false;
            }
            ipaddr += "/" + std::to_string(count);
            yaml["network"]["ethernets"][device]["addresses"].SetStyle(YAML::EmitterStyle::Flow);
            yaml["network"]["ethernets"][device]["addresses"][0] = ipaddr;
            if (!ipaddr6.empty() && !subnetlength.empty() && !dhcp6)
            {
                ipaddr6 += "/" + subnetlength;
                yaml["network"]["ethernets"][device]["addresses"][1] = ipaddr6;
            }
            else
            {
                yaml["network"]["ethernets"][device]["addresses"].remove(1);
            }

            if (!gateway.empty())
            {
                yaml["network"]["ethernets"][device]["gateway4"] = gateway;
            }
            else
            {
                yaml["network"]["ethernets"][device].remove("gateway4");
            }

            if (!gateway6.empty() && !dhcp6)
            {
                yaml["network"]["ethernets"][device]["gateway6"] = gateway6;
            }
            else
            {
                yaml["network"]["ethernets"][device].remove("gateway6");
            }

            if (!dnsprimary.empty())
            {
                yaml["network"]["ethernets"][device]["nameservers"]["addresses"].SetStyle(YAML::EmitterStyle::Flow);
                yaml["network"]["ethernets"][device]["nameservers"]["addresses"][0] = dnsprimary;
                if (!dnssecondary.empty())
                {
                    yaml["network"]["ethernets"][device]["nameservers"]["addresses"][1] = dnssecondary;
                }
                else
                {
                    yaml["network"]["ethernets"][device]["nameservers"]["addresses"].remove(1);
                }
            }
        }
        else
        {
            yaml["network"]["ethernets"].remove(device);
        }
    }
    std::ofstream fout("/etc/netplan/01-network-manager-all.yaml");
    fout << yaml << "\n";
    fout.close();
    pid_t status = system("netplan apply");
    return (status != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return true;
}

bool NetworkManager::getallNetwork(json &retvalue)
{
    bool empty = false;
    retvalue = json::array();
    std::string result;
    if (GetResultFromSystemCall("ip -j a", result) < 0)
    {
        empty = true;
    }
    try
    {
        auto ipInfoJson = json::parse(result);
        if (ipInfoJson.is_array())
        {
            for (auto i : ipInfoJson)
            {
                std::string mac;
                std::string ifname = i["ifname"].get<std::string>();
                if (ifname == "lo" || boost::starts_with(ifname, "docker") || boost::starts_with(ifname, "virbr"))
                {
                    continue;
                }
                mac = i["address"].get<std::string>();

                json networkJson;
                networkJson["device"] = ifname;
                networkJson["mac"] = mac;
                std::string address4, address6;
                int subnetlength4 = 0;
                int subnetlength6 = 0;
                json addrInfo = i["addr_info"];
                if (addrInfo.is_array() && addrInfo.size() > 0)
                {
                    for (auto &j : addrInfo)
                    {
                        std::string family = j["family"].get<std::string>();
                        if (family == "inet")
                        {
                            address4 = j["local"].get<std::string>();
                            subnetlength4 = j["prefixlen"].get<int>();
                        }
                        else if (family == "inet6")
                        {
                            address6 = j["local"].get<std::string>();
                            subnetlength6 = j["prefixlen"].get<int>();
                        }
                    }
                }
                else
                {
                    std::string ip4 = getIpAddr(ifname.c_str(), IPType::IPV4);
                    std::string ip6 = getIpAddr(ifname.c_str(), IPType::IPV6);
                    if (!ip4.empty())
                    {
                        address4 = ip4.substr(0, ip4.find("/"));
                        subnetlength4 = std::atoi(ip4.substr(ip4.find("/") + 1).c_str());
                    }
                    if (!ip6.empty())
                    {
                        address6 = ip6.substr(0, ip6.find("/"));
                        subnetlength6 = std::atoi(ip6.substr(ip6.find("/") + 1).c_str());
                    }
                }
                networkJson["ipv4"]["address"] = address4;
                networkJson["ipv4"]["subnetlength"] = subnetlength4;
                networkJson["ipv6"]["address"] = address6;
                networkJson["ipv6"]["subnetlength"] = subnetlength6;
                retvalue.push_back(networkJson);
            }
        }
        else
        {
            empty = true;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    if (empty)
    {
        json networkJson;
        networkJson["device"] = "";
        networkJson["mac"] = "";
        networkJson["ipv4"]["address"] = "";
        networkJson["ipv4"]["subnetlength"] = 0;
        networkJson["ipv6"]["address"] = "";
        networkJson["ipv6"]["subnetlength"] = 0;
        retvalue.push_back(networkJson);
    }
    return true;
}

// int main(int argc, const char **argv)
// {
//     NetworkManager nm;
//     json info;
//     nm.getNetwork("eno1", info);
//     std::cout << "getNetwork: " << info.dump() << std::endl;
//     json info2;
//     nm.getallNetwork(info2);
//     std::cout << "getallNetwork: " << info2.dump() << std::endl;

//     json info3;
//     std::string str = "{                \
//     \"device\": \"eno1\",               \
//     \"mac\": \"0:0:0:0:0:0\",           \
//     \"ipv4\": {                         \
//         \"dhcp\": false,                \
//         \"address\": \"10.10.10.10\",   \
//         \"netmask\": \"255.255.0.0\",   \
//         \"gateway\": \"10.10.0.1\"      \
//     },                                  \
//     \"ipv6\": {                         \
//         \"dhcp\": false,                \
//         \"address\": \"fe00::2\",       \
//         \"subnetlength\": \"64\",       \
//         \"gateway\": \"fe00::1\"        \
//     },                                  \
//     \"dns\": {                          \
//         \"primary\": \"8.8.8.8\",       \
//         \"secondary\": \"2.2.2.2\"      \
//     }                                   \
// }";
//     std::cout << "str: " << str << std::endl;
//     info3 = json::parse(str);
//     bool ret = nm.setNetwork(info3);
//     return 0;
// }