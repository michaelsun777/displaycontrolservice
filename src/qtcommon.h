#ifndef _QT_COMMON_H
#define _QT_COMMON_H

#include <QMetaType>
#include "../3rd/json/include/nlohmann/json.hpp"

struct QtDlgInfo
{
public:
    std::string name;
    std::string dlgId;
    int type;//dlg type
    std::string url;
    std::string path;
    std::string param;
    int posType;//1需要转换比例，2真实屏幕坐标
    int xVirtual;
    int yVirtual;
    int xPos;
    int yPos;
    int width;
    int height;
    bool titleEnable; //是否显示标题
    int order;
    void operator=(QtDlgInfo & src)
    {
        name = src.name.c_str();
        dlgId = src.dlgId.c_str();
        url = src.url.c_str();
        path = src.path.c_str();
        param = src.param.c_str();
        type = src.type;
        posType = src.posType;
        xVirtual = src.xVirtual;
        yVirtual = src.yVirtual;
        xPos = src.xPos;
        yPos = src.yPos;
        width = src.width;
        height = src.height;
        titleEnable = src.titleEnable;
        if(src.order != 0)     
            order = src.order;
    }

    bool check(nlohmann::json jdata,std::string strError)
    {
        if (jdata.find("dlgId") != jdata.end())
        {
            dlgId = jdata["dlgId"].template get<std::string>();
        }
        else
        {            
            strError = "lost dlgid!";
            return false;
        }

        if (jdata.find("name") != jdata.end())
        {
            name = jdata["name"].template get<std::string>();
        }

        if (jdata.find("url") != jdata.end())
        {
            url = jdata["url"].template get<std::string>();
        }
        if (jdata.find("path") != jdata.end())
        {
            path = jdata["path"].template get<std::string>();
        }

        if (jdata.find("param") != jdata.end())
        {
            param = jdata["param"].template get<std::string>();
        }

        if (jdata.find("type") != jdata.end())
        {
            type = jdata["type"].template get<int>();
        }

        if (jdata.find("posType") != jdata.end())
        {
            posType = jdata["posType"].template get<int>();
        }

        if (jdata.find("xVirtual") != jdata.end())
        {
            xVirtual = jdata["xVirtual"].template get<int>();
        }

        if (jdata.find("yVirtual") != jdata.end())
        {
            yVirtual = jdata["yVirtual"].template get<int>();
        }

        if (jdata.find("xPos") != jdata.end())
        {
            xPos = jdata["xPos"].template get<int>();
        }
        else
        {
            strError = "lost xPos!";
            return false;
        }

        if (jdata.find("yPos") != jdata.end())
        {
            yPos = jdata["yPos"].template get<int>();
        }
        else
        {
            strError = "lost yPos!";
            return false;
        }

        if (jdata.find("height") != jdata.end())
        {
            height = jdata["height"].template get<int>();
        }
        else
        {
            strError = "lost height!";
            return false;
        }

        if (jdata.find("width") != jdata.end())
        {
            width = jdata["width"].template get<int>();
        }
        else
        {
            strError = "lost width!";
            return false;
        }

        if (jdata.find("order") != jdata.end())
        {
            order = jdata["order"].template get<int>();
        }
        else
        {
            order = 999;
        }

        return true;
    }

    bool toJson(nlohmann::json & jdata)
    {
        try
        {
            jdata["name"] = name;
            jdata["id"] = dlgId;
            jdata["url"] = url;
            jdata["path"] = path;
            jdata["param"] = param;
            jdata["type"] = type;
            jdata["posType"] = posType;
            jdata["xVirtual"] = xVirtual;
            jdata["yVirtual"] = yVirtual;
            jdata["xPos"] = xPos;
            jdata["yPos"] = yPos;
            jdata["height"] = height;
            jdata["width"] = width;
            jdata["order"] = order;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool jsonToDlgInfo(nlohmann::json jdata,std::string strError)
    {
        try
        {
            if(check(jdata,strError))
                return true;
            else
                return false;
        }
        catch(...)
        {
            return false;
        }

    }

    bool operator <(const QtDlgInfo& src) const // 升序排序时必须写的函数
    {
        return order < src.order;
    }
    bool operator >(const QtDlgInfo& src) const // 降序排序时必须写的函数
    {
        return order > src.order;
    }

};

Q_DECLARE_METATYPE(QtDlgInfo)




#endif