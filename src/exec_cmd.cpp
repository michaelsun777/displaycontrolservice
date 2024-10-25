#include "exec_cmd.h"

namespace CMDEXEC
{
    const int MAX_COMMAND_SIZE = 4096;
    const int MAX_COMMAND_ARGS_COUNT = 127;

    map<string, string> StepValues;
    map<string, FUNCTION *> StepFuncs;

    void CommandConvert(char *buffer, char *args[])
    {
        int size = strlen(buffer) + 1;
        int i = 0, j = 0;
        char *head = buffer;
        while (i < size)
        {
            if (head[0] == '"')
            {
                if (buffer[i] == '"' && head != buffer + i)
                {
                    args[j++] = head;
                    buffer[i + 1] = '\0';
                    i += 2;
                    head = buffer + i;
                    continue;
                }
                i++;
                continue;
            }

            if (buffer[i] == ' ' || buffer[i] == '\0')
            {
                args[j++] = head;
                buffer[i] = '\0';
                i++;
                head = buffer + i;
            }
            i++;
        }
    }

    bool Execute(const std::string &cmd, CmdRes &res)
    {
        try
        {
            assert(cmd.size() < MAX_COMMAND_SIZE);
            int fd_out[2];
            int fd_err[2];
            int ret = pipe(fd_out);
            if (ret == -1)
            {
                return false;
            }
            ret = pipe(fd_err);
            if (ret == -1)
            {
                return false;
            }
            pid_t pid = fork();
            if (pid == 0)
            {
                close(fd_out[0]);
                close(fd_err[0]);
                dup2(fd_out[1], STDOUT_FILENO);
                dup2(fd_err[1], STDERR_FILENO);
                char buffer[MAX_COMMAND_SIZE] = {0};
                strcpy(buffer, cmd.c_str());
                char *args[MAX_COMMAND_ARGS_COUNT] = {nullptr};
                CommandConvert(buffer, args);
                execvp(args[0], args);
                perror("execvp");
                return -1;
            }
            else if (pid > 0)
            {
                res.StdoutString.clear();
                res.StdoutString.clear();
                close(fd_out[1]);
                close(fd_err[1]);
                char buffer[1024] = {0};
                while (true)
                {
                    int len = read(fd_out[0], buffer, sizeof(buffer));
                    if (len == 0)
                    {
                        break;
                    }
                    res.StdoutString.append(std::string(buffer, len));
                }
                while (true)
                {
                    int len = read(fd_err[0], buffer, sizeof(buffer));
                    if (len == 0)
                    {
                        break;
                    }
                    res.StderrString.append(std::string(buffer, len));
                }
                close(fd_err[0]);
                close(fd_out[0]);
                int status = 0;
                wait(&status);
                res.ExitCode = WEXITSTATUS(status);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void Stringsplit(std::string str, char split, std::vector<std::string> &res)
    {
        std::istringstream iss(str);       // 输入流
        std::string token;                 // 接收缓冲区
        while (getline(iss, token, split)) // 以split为分隔符
        {
            res.push_back(token);
        }
    }

    std::string Node::GetValue()
    {
        if (NodeType == NodeType::CONST_VALUE)
        {
            return NodeName;
        }
        else if (NodeType == NodeType::FIELD)
        {
            return StepValues[NodeName];
        }
        else if (NodeType == NodeType::FUNCTION_TYPE)
        {
            return StepFuncs[NodeName](Children);
        }
        return "";
    }

    bool StartsWith(std::string src, std::string sub)
    {
        return src.find(sub) == 0;
    }
    bool EndsWith(std::string src, std::string sub)
    {
        return src.rfind(sub) == (src.length() - sub.length());
    }

    bool StartsWith(std::string src, char c)
    {
        return src.find(c) == 0;
    }
    bool EndsWith(std::string src, char c)
    {
        return src.rfind(c) == (src.length() - 1);
    }

    std::string Lstrip(const std::string &s)
    {
        if (s.front() != ' ')
            return s;
        for (auto index = 0u; index < s.size(); index++)
        {
            if (s[index] != ' ')
            {
                return s.substr(index);
            }
        }
        return s;
    }

    std::string Lstrip(const std::string &s,char c)
    {
        if (s.front() != c)
            return s;
        for (auto index = 0u; index < s.size(); index++)
        {
            if (s[index] != c)
            {
                return s.substr(index);
            }
        }
        return s;
    }

    std::string Rstrip(const std::string &s)
    {
        if (s.back() != ' ')
            return s;
        for (auto index = s.size() - 1; index >= 0; index--)
        {
            if (s[index] != ' ')
            {
                return s.substr(0, index + 1);
            }
        }
        return s;
    }

    std::string Rstrip(const std::string &s,char c)
    {
        if (s.back() != c)
            return s;
        for (auto index = s.size() - 1; index >= 0; index--)
        {
            if (s[index] != c)
            {
                return s.substr(0, index + 1);
            }
        }
        return s;
    }

    std::string Strip(const std::string &s)
    {
        string newString = Lstrip(s);
        return Rstrip(newString);
    }

    std::string StripQuote(const std::string &src)
    {
        if (StartsWith(src, "\"") && EndsWith(src, "\""))
        {
            return src.substr(1, src.size() - 2);
        }
        return src;
    }

    std::string StripBrackets(const std::string &src,char front,char back)
    {
       
        if (StartsWith(src, front) && EndsWith(src, back))
        {
            return src.substr(1, src.size() - 2);
        }
        return src;
    }

    std::vector<std::string> Split(const std::string &s, const std::string &separator)
    {
        vector<string> result;
        int pos;
        string temp = s;
        while ((pos = temp.find_first_of(separator)) != temp.npos)
        {
            if (pos > 0)
            {
                result.push_back(temp.substr(0, pos));
            }
            temp = temp.substr(pos + separator.length());
        }
        if (temp.length() > 0)
        {
            result.push_back(temp);
        }
        return std::move(result);
    }
    std::vector<std::string> Split(const std::string &s, char separator)
    {
        vector<string> result;
        int pos;
        string temp = s;
        while ((pos = temp.find_first_of(separator)) != temp.npos)
        {
            if (pos > 0)
            {
                result.push_back(temp.substr(0, pos));
            }
            temp = temp.substr(pos + 1);
        }
        if (temp.length() > 0)
        {
            result.push_back(temp);
        }
        return std::move(result);
    }

    string replaceAll(string &str, string oldStr, string newStr)
    {
        string::size_type pos = str.find(oldStr);
        while (pos != string::npos)
        {
            str.replace(pos, oldStr.size(), newStr);
            pos = str.find(oldStr);
        }
        return str;
    }

    std::string GetParamString(const std::string &src, size_t &endIndex)
    {
        int start = 0;
        for (size_t index = 0; index < src.size(); index++)
        {
            if (src[index] == '(')
            {
                start = index + 1;
                break;
            }
        }
        auto count = 1u;
        for (size_t index = start + 1; index < src.size(); index++)
        {
            if (src[index] == '(')
            {
                count++;
                continue;
            }
            if (src[index] == ')')
            {
                count--;
                if (count == 0)
                {
                    endIndex = index;
                    break;
                }
                continue;
            }
        }
        return src.substr(start, endIndex - start);
    }
    std::string GetNextNodeString(const std::string &src, size_t &endIndex, char op)
    {
        if (StartsWith(src, "$STEP"))
        {
            endIndex = src.find(op, 1);
            return src.substr(0, endIndex);
        }
        if (StartsWith(src, "$"))
        {
            GetParamString(src, endIndex);
            endIndex += 1;
            return src.substr(0, endIndex);
        }
        endIndex = src.find(op);
        return src.substr(0, endIndex);
    }
    std::string GetTopLevelNextNodeString(const std::string &src, size_t &endIndex)
    {
        return GetNextNodeString(src, endIndex, '$');
    }
    std::string GetParamNextNodeString(const std::string &src, size_t &endIndex)
    {
        return GetNextNodeString(src, endIndex, ',');
    }

    void ParseParam(std::string src, Node *parent)
    {
        size_t index;
        while (!src.empty())
        {
            Node *node = new Node();
            parent->Children.push_back(node);
            auto dest = GetParamNextNodeString(src, index);
            ParseNode(dest, node);
            if (index == src.npos || index >= src.size())
            {
                src = "";
            }
            else
            {
                src = src.substr(index + 1);
                src = Lstrip(src);
            }
        }
    }

    void PrintNode(Node* head, int depth)
    {
        string indent(depth * 2, '-');
        printf("%sNodeType[%d], NodeName[%s]\n", indent.c_str(), head->NodeType, head->NodeName.c_str());
        depth++;
        for (auto node : head->Children)
        {
            PrintNode(node, depth);
        }
    }
    void ParseNode(std::string src, Node *node)
    {
        src = Strip(src);
        src = StripQuote(src);
        if (StartsWith(src, "$STEP"))
        {
            node->NodeType = NodeType::FIELD;
            node->NodeName = src.substr(1);
        }
        else if (StartsWith(src, "$"))
        {
            node->NodeType = NodeType::FUNCTION_TYPE;
            auto index = src.find('(');
            node->NodeName = src.substr(1, index - 1);
            size_t endIndex;
            auto paramString = GetParamString(src, endIndex);
            ParseParam(paramString, node);
        }
        else
        {
            node->NodeType = NodeType::CONST_VALUE;
            node->NodeName = src;
        }
    }
    std::string ParseSystemInfo(std::string src)
    {
        std::vector<Node *> headNodes;
        size_t index;
        src = Strip(src);
        while (src.size() > 0)
        {
            auto dest = GetTopLevelNextNodeString(src, index);
            src = src.substr(index);

            Node *head = new Node();
            ParseNode(dest, head);
            headNodes.push_back(head);
        }
        for (auto node : headNodes)
        {
            PrintNode(node, 0);
        }

        string result = "";
        for (auto node : headNodes)
        {
            result += node->GetValue();
        }
        return result;
    }
}