#include "IniReader.h"
#include <algorithm>
#include <cctype>

CIniReader::CIniReader()
{
    SetIniPath();
}

CIniReader::CIniReader(std::filesystem::path szFileName)
{
    SetIniPath(szFileName);
}

bool CIniReader::operator==(CIniReader &ir)
{
    auto &a = m_ini;
    auto &b = ir.m_ini;
    for (auto const &it : a)
    {
        auto const &section = std::get<0>(it);
        auto const &collection = std::get<1>(it);
        if (collection.size() != b[section].size())
        {
            return false;
        }
        for (auto const &it2 : collection)
        {
            auto const &key = std::get<0>(it2);
            auto const &value = std::get<1>(it2);
            if (value != b[section][key])
            {
                return false;
            }
        }
    }
    return a.size() == b.size();
}

bool CIniReader::operator!=(CIniReader &ir)
{
    return !(*this == ir);
}

bool CIniReader::CompareBySections(CIniReader &ir)
{
    std::vector<std::string> sections1;
    std::vector<std::string> sections2;

    for (auto const &it : m_ini)
        sections1.emplace_back(std::get<0>(it));

    for (auto const &it : ir.m_ini)
        sections2.emplace_back(std::get<0>(it));

    return std::equal(sections1.begin(), sections1.end(), sections2.begin(), sections2.end());
}

bool CIniReader::CompareByValues(CIniReader &ir)
{
    return *this == ir;
}

const std::filesystem::path &CIniReader::GetIniPath()
{
    return m_szFileName;
}

void CIniReader::SetNewIniPathForSave(std::filesystem::path szFileName)
{
    m_szFileName = szFileName;
}

void CIniReader::SetIniPath()
{
    SetIniPath("");
}

void CIniReader::SetIniPath(std::filesystem::path szFileName)
{
    std::filesystem::path modulePath(szFileName);

    if (szFileName.is_absolute())
    {
        m_szFileName = szFileName;
    }
    else if (szFileName.empty())
    {
        m_szFileName = modulePath.replace_extension(".ini");
    }
    else
    {
        m_szFileName = modulePath.parent_path() / szFileName;
    }

    mINI::INIFile file(m_szFileName);
    file.read(m_ini);
}

int CIniReader::ReadInteger(std::string_view szSection, std::string_view szKey, int iDefaultValue)
{
    try
    {
        if (m_ini.size() && m_ini.has(szSection.data()))
        {
            auto &collection = m_ini[szSection.data()];
            if (collection.has(szKey.data()))
            {
                auto &value = collection[szKey.data()];
                return std::stoi(value, nullptr, 10);
            }
        }
    }
    catch (...)
    {
    }
    return iDefaultValue;
}

float CIniReader::ReadFloat(std::string_view szSection, std::string_view szKey, float fltDefaultValue)
{
    try
    {
        if (m_ini.size() && m_ini.has(szSection.data()))
        {
            auto &collection = m_ini[szSection.data()];
            if (collection.has(szKey.data()))
            {
                auto &value = collection[szKey.data()];
                return static_cast<float>(std::atof(value.data()));
            }
        }
    }
    catch (...)
    {
    }
    return fltDefaultValue;
}

bool CIniReader::ReadBoolean(std::string_view szSection, std::string_view szKey, bool bolDefaultValue)
{
    try
    {
        if (m_ini.size() && m_ini.has(szSection.data()))
        {
            auto &collection = m_ini[szSection.data()];
            if (collection.has(szKey.data()))
            {
                auto value = collection[szKey.data()];
                if (value.size() == 1)
                    return value != "0";
                else
                {
                    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                    if (value == "false")
                        return false;
                    else if (value == "true")
                        return true;
                }
            }
        }
    }
    catch (...)
    {
    }
    return bolDefaultValue;
}

std::string CIniReader::ReadString(std::string_view szSection, std::string_view szKey, std::string_view szDefaultValue)
{
    try
    {
        if (m_ini.size() && m_ini.has(szSection.data()))
        {
            auto &collection = m_ini[szSection.data()];
            if (collection.has(szKey.data()))
            {
                auto value = collection[szKey.data()];
                if (!value.empty())
                {
                    if (value.at(0) == '\"' || value.at(0) == '\'')
                        value.erase(0, 1);
                    if (value.at(value.size() - 1) == '\"' || value.at(value.size() - 1) == '\'')
                        value.erase(value.size() - 1);
                }
                return value;
            }
        }
    }
    catch (...)
    {
    }
    return std::string(szDefaultValue);
}

void CIniReader::WriteInteger(std::string_view szSection, std::string_view szKey, int iValue, bool pretty)
{
    try
    {
        mINI::INIFile file(m_szFileName);
        m_ini[szSection.data()][szKey.data()] = std::to_string(iValue);
        file.write(m_ini, pretty);
    }
    catch (...)
    {
    }
}

void CIniReader::WriteFloat(std::string_view szSection, std::string_view szKey, float fltValue, bool pretty)
{
    try
    {
        mINI::INIFile file(m_szFileName);
        m_ini[szSection.data()][szKey.data()] = std::to_string(fltValue);
        file.write(m_ini, pretty);
    }
    catch (...)
    {
    }
}

void CIniReader::WriteBoolean(std::string_view szSection, std::string_view szKey, bool bolValue, bool pretty)
{
    try
    {
        mINI::INIFile file(m_szFileName);
        m_ini[szSection.data()][szKey.data()] = bolValue ? "True" : "False";
        file.write(m_ini, pretty);
    }
    catch (...)
    {
    }
}

void CIniReader::WriteString(std::string_view szSection, std::string_view szKey, std::string_view szValue, bool pretty)
{
    try
    {
        mINI::INIFile file(m_szFileName);
        m_ini[szSection.data()][szKey.data()] = szValue.data();
        file.write(m_ini, pretty);
    }
    catch (...)
    {
    }
}