#include "ini.h"

namespace mINI
{



    std::vector<std::string> INIReader::readFile()
    {
        fileReadStream.seekg(0, std::ios::end);
        const std::size_t fileSize = static_cast<std::size_t>(fileReadStream.tellg());
        fileReadStream.seekg(0, std::ios::beg);
        if (fileSize >= 3)
        {
            const char header[3] = {
                static_cast<char>(fileReadStream.get()),
                static_cast<char>(fileReadStream.get()),
                static_cast<char>(fileReadStream.get())};
            isBOM = (header[0] == static_cast<char>(0xEF) &&
                     header[1] == static_cast<char>(0xBB) &&
                     header[2] == static_cast<char>(0xBF));
        }
        else
        {
            isBOM = false;
        }
        std::string fileContents;
        fileContents.resize(fileSize);
        fileReadStream.seekg(isBOM ? 3 : 0, std::ios::beg);
        fileReadStream.read(&fileContents[0], fileSize);
        fileReadStream.close();
        std::vector<std::string> output;
        if (fileSize == 0)
        {
            return output;
        }
        std::string buffer;
        buffer.reserve(50);
        for (std::size_t i = 0; i < fileSize; ++i)
        {
            char &c = fileContents[i];
            if (c == '\n')
            {
                output.emplace_back(buffer);
                buffer.clear();
                continue;
            }
            if (c != '\0' && c != '\r')
            {
                buffer += c;
            }
        }
        output.emplace_back(buffer);
        return output;
    }

    INIReader::INIReader(std::filesystem::path const &filename, bool keepLineData)
    {
        fileReadStream.open(filename, std::ios::in | std::ios::binary);
        if (keepLineData)
        {
            lineData = std::make_shared<std::vector<std::string>>();
        }
    }
    INIReader::~INIReader() {}

    bool INIReader::operator>>(INIStructure &data)
    {
        if (!fileReadStream.is_open())
        {
            return false;
        }
        std::vector<std::string> fileLines = readFile();
        std::string section;
        bool inSection = false;
        INIParser::T_ParseValues parseData;
        for (auto const &line : fileLines)
        {
            auto parseResult = INIParser::parseLine(line, parseData);
            if (parseResult == INIParser::PDataType::PDATA_SECTION)
            {
                inSection = true;
                data[section = std::get<0>(parseData)];
            }
            else if (inSection && (parseResult == INIParser::PDataType::PDATA_KEYVALUE_COMMENT ||
                                   parseResult == INIParser::PDataType::PDATA_NONE || parseResult == INIParser::PDataType::PDATA_COMMENT))
            {
                auto const &key = std::get<0>(parseData);
                auto const &value = std::get<1>(parseData);
                auto const &comment = std::get<2>(parseData);
                data[section].set(key, value, comment, true);
            }
            if (lineData && parseResult != INIParser::PDataType::PDATA_UNKNOWN)
            {
                if (parseResult == INIParser::PDataType::PDATA_KEYVALUE_COMMENT && !inSection)
                {
                    continue;
                }
                lineData->emplace_back(line);
            }
        }
        return true;
    }
    std::shared_ptr<std::vector<std::string>> INIReader::getLines()
    {
        return lineData;
    }





    INIGenerator::INIGenerator(std::filesystem::path const &filename)
    {
        fileWriteStream.open(filename, std::ios::out | std::ios::binary);
    }
    INIGenerator::~INIGenerator() {}

    bool INIGenerator::operator<<(INIStructure const &data)
    {
        if (!fileWriteStream.is_open())
        {
            return false;
        }
        if (!data.size())
        {
            return true;
        }
        auto it = data.begin();
        for (;;)
        {
            auto const &section = std::get<0>(*it);
            auto const &collection = std::get<1>(*it);
            fileWriteStream
                << "["
                << section
                << "]";
            if (collection.size())
            {
                fileWriteStream << INIStringUtil::endl;
                auto it2 = collection.begin();
                for (;;)
                {
                    auto key = std::get<0>(*it2);
                    INIStringUtil::replace(key, "=", "\\=");
                    auto value = std::get<1>(*it2);
                    INIStringUtil::trim(value);
                    auto comment = std::get<2>(*it2);
                    fileWriteStream
                        << key
                        << ((key.length() || value.length()) ? ((prettyPrint) ? " = " : "=") : "")
                        << value
                        << comment;
                    if (++it2 == collection.end())
                    {
                        break;
                    }
                    fileWriteStream << INIStringUtil::endl;
                }
            }
            if (++it == data.end())
            {
                break;
            }
            fileWriteStream << INIStringUtil::endl;
            if (prettyPrint)
            {
                fileWriteStream << INIStringUtil::endl;
            }
        }
        return true;
    }

    std::vector<std::string> INIWriter::getLazyOutput(std::shared_ptr<std::vector<std::string>> const &lineData, INIStructure &data, INIStructure &original)
    {
        std::vector<std::string> output;
        INIParser::T_ParseValues parseData;
        std::string sectionCurrent;
        bool parsingSection = false;
        bool continueToNextSection = false;
        bool discardNextEmpty = false;
        bool writeNewKeys = false;
        std::size_t lastKeyLine = 0;
        for (auto line = lineData->begin(); line != lineData->end(); ++line)
        {
            if (!writeNewKeys)
            {
                auto parseResult = INIParser::parseLine(*line, parseData);
                if (parseResult == INIParser::PDataType::PDATA_SECTION)
                {
                    if (parsingSection)
                    {
                        writeNewKeys = true;
                        parsingSection = false;
                        --line;
                        continue;
                    }
                    sectionCurrent = std::get<0>(parseData);
                    if (data.has(sectionCurrent))
                    {
                        parsingSection = true;
                        continueToNextSection = false;
                        discardNextEmpty = false;
                        output.emplace_back(*line);
                        lastKeyLine = output.size();
                    }
                    else
                    {
                        continueToNextSection = true;
                        discardNextEmpty = true;
                        continue;
                    }
                }
                else if (parseResult == INIParser::PDataType::PDATA_KEYVALUE_COMMENT)
                {
                    if (continueToNextSection)
                    {
                        continue;
                    }
                    if (data.has(sectionCurrent))
                    {
                        auto &collection = data[sectionCurrent];
                        auto const &key = std::get<0>(parseData);
                        auto const &value = std::get<1>(parseData);
                        auto const &comment = std::get<2>(parseData);
                        if (collection.has(key))
                        {
                            auto outputValue = collection[key];
                            auto outputComment = collection.getComment(key);
                            if (value == outputValue || collection.count(key) > 1)
                            {
                                output.emplace_back(*line);
                            }
                            else
                            {
                                INIStringUtil::trim(outputValue);
                                auto lineNorm = *line;
                                INIStringUtil::replace(lineNorm, "\\=", "  ");
                                auto equalsAt = lineNorm.find_first_of('=');
                                auto valueAt = lineNorm.find_first_not_of(
                                    INIStringUtil::whitespaceDelimiters,
                                    equalsAt + 1);
                                std::string outputLine = line->substr(0, valueAt);
                                if (prettyPrint && equalsAt + 1 == valueAt)
                                {
                                    outputLine += " ";
                                }
                                outputLine += outputValue;

                                if (!outputComment.empty() && outputComment.front() == ' ')
                                {
                                    std::ptrdiff_t i = outputValue.size() - value.size();
                                    if (i < 0)
                                    {
                                        while (i < 0)
                                        {
                                            outputComment.insert(0, " ");
                                            i++;
                                        }
                                    }
                                    else if (i > 0)
                                    {
                                        auto pos = outputComment.find_first_not_of(' ');
                                        while (i > 0 && pos >= size_t(i))
                                        {
                                            if (outputComment.front() == ' ')
                                                outputComment.erase(0, 1);
                                            i--;
                                        }
                                    }
                                }
                                outputLine += outputComment;
                                output.emplace_back(outputLine);
                            }
                            lastKeyLine = output.size();
                        }
                    }
                }
                else
                {
                    if (discardNextEmpty && line->empty())
                    {
                        discardNextEmpty = false;
                    }
                    else if (parseResult != INIParser::PDataType::PDATA_UNKNOWN)
                    {
                        output.emplace_back(*line);
                    }
                }
            }
            if (writeNewKeys || std::next(line) == lineData->end())
            {
                std::vector<std::string> linesToAdd;
                if (data.has(sectionCurrent) && original.has(sectionCurrent))
                {
                    auto const &collection = data[sectionCurrent];
                    auto const &collectionOriginal = original[sectionCurrent];
                    for (auto const &it : collection)
                    {
                        auto key = std::get<0>(it);
                        if (collectionOriginal.has(key))
                        {
                            continue;
                        }
                        auto value = std::get<1>(it);
                        INIStringUtil::replace(key, "=", "\\=");
                        INIStringUtil::trim(value);
                        linesToAdd.emplace_back(
                            key + ((prettyPrint) ? " = " : "=") + value);
                    }
                }
                if (!linesToAdd.empty())
                {
                    output.insert(
                        output.begin() + lastKeyLine,
                        linesToAdd.begin(),
                        linesToAdd.end());
                }
                if (writeNewKeys)
                {
                    writeNewKeys = false;
                    --line;
                }
            }
        }
        for (auto const &it : data)
        {
            auto const &section = std::get<0>(it);
            if (original.has(section))
            {
                continue;
            }
            if (prettyPrint && output.size() > 0 && !output.back().empty())
            {
                output.emplace_back();
            }
            output.emplace_back("[" + section + "]");
            auto const &collection = std::get<1>(it);
            for (auto const &it2 : collection)
            {
                auto key = std::get<0>(it2);
                auto value = std::get<1>(it2);
                INIStringUtil::replace(key, "=", "\\=");
                INIStringUtil::trim(value);
                output.emplace_back(
                    key + ((prettyPrint) ? " = " : "=") + value);
            }
        }
        return output;
    }

    INIWriter::INIWriter(std::filesystem::path const &filename) : filename(filename)
    {
    }
    INIWriter::~INIWriter() {}

    bool INIWriter::operator<<(INIStructure &data)
    {
        std::error_code ec;
        if (!std::filesystem::exists(filename, ec))
        {
            INIGenerator generator(filename);
            generator.prettyPrint = prettyPrint;
            return generator << data;
        }
        INIStructure originalData;
        std::shared_ptr<std::vector<std::string>> lineData;
        bool readSuccess = false;
        bool fileIsBOM = false;
        {
            INIReader reader(filename, true);
            if ((readSuccess = reader >> originalData))
            {
                lineData = reader.getLines();
                fileIsBOM = reader.isBOM;
            }
        }
        if (!readSuccess)
        {
            return false;
        }
        std::vector<std::string> output = getLazyOutput(lineData, data, originalData);
        std::ofstream fileWriteStream(filename, std::ios::out | std::ios::binary);
        if (fileWriteStream.is_open())
        {
            if (fileIsBOM)
            {
                const char utf8_BOM[3] = {
                    static_cast<char>(0xEF),
                    static_cast<char>(0xBB),
                    static_cast<char>(0xBF)};
                fileWriteStream.write(utf8_BOM, 3);
            }
            if (output.size())
            {
                auto line = output.begin();
                for (;;)
                {
                    fileWriteStream << *line;
                    if (++line == output.end())
                    {
                        break;
                    }
                    fileWriteStream << INIStringUtil::endl;
                }
            }
            return true;
        }
        return false;
    }










    INIFile::INIFile(std::filesystem::path const &filename) : _filename(filename)
    {
    }

    INIFile::~INIFile() {}

    bool INIFile::read(INIStructure &data) const
    {
        if (data.size())
        {
            data.clear();
        }
        if (_filename.empty())
        {
            return false;
        }
        INIReader reader(_filename);
        return reader >> data;
    }
    bool INIFile::generate(INIStructure const &data, bool pretty) const
    {
        if (_filename.empty())
        {
            return false;
        }
        INIGenerator generator(_filename);
        generator.prettyPrint = pretty;
        return generator << data;
    }
    bool INIFile::write(INIStructure &data, bool pretty) const
    {
        if (_filename.empty())
        {
            return false;
        }
        INIWriter writer(_filename);
        writer.prettyPrint = pretty;
        return writer << data;
    }
}