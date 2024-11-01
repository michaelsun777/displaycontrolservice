#ifndef _QT_COMMON_H
#define _QT_COMMON_H

#include <QMetaType>

struct QtDlgInfo
{
public:
    std::string name;
    std::string dlgId;
    int type;//dlg type
    std::string url;
    std::string path;
    std::string param;
    int postype;//1需要转换比例，2真实屏幕坐标
    int xVirtual;
    int yVirtual;
    int xPos;
    int yPos;
    int width;
    int height;
    bool titleEnable; //是否显示标题
    void operator=(QtDlgInfo & src)
    {
        name = src.name.c_str();
        dlgId = src.dlgId.c_str();
        url = src.url.c_str();
        path = src.path.c_str();
        param = src.param.c_str();
        type = src.type;
        postype = src.postype;
        xVirtual = src.xVirtual;
        yVirtual = src.yVirtual;
        xPos = src.xPos;
        yPos = src.yPos;
        width = src.width;
        height = src.height;
        titleEnable = src.titleEnable;
    }

};

Q_DECLARE_METATYPE(QtDlgInfo)




#endif