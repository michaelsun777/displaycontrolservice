#include "CefViewSchemeHandlerFactory.h"

#include <CefViewBrowserClient.h>
#include <CefViewBrowserClientDelegate.h>

#include "CefViewSchemeHandler.h"
#include "include/wrapper/cef_stream_resource_handler.h"

CefViewSchemeHandlerFactory::CefViewSchemeHandlerFactory(CefRefPtr<CefViewBrowserApp> app)
  : app_(app)
{
}

CefViewSchemeHandlerFactory::~CefViewSchemeHandlerFactory() {}

CefRefPtr<CefResourceHandler>
CefViewSchemeHandlerFactory::Create(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& scheme_name,
                                    CefRefPtr<CefRequest> request)
{
  std::string name = scheme_name.ToString();
  if (name == "cefview") {
    std::string url = request->GetURL().ToString();
    std::string urlPrefix = name;
    urlPrefix += "://";
    url.erase(0, urlPrefix.size());
    std::string backgroundColor = "skyblue";
    std::string fontColor = "green";
    static std::string s;
    s = "<!DOCTYPE html><html><head><meta "
        "charset=\"UTF-8\"><script>resetrem();window.addEventListener(\"orientationchange\", "
        "resetrem);window.addEventListener(\"resize\",resetrem);function resetrem(){var html = "
        "document.querySelector(\"html\");var width = html.getBoundingClientRect().width;html.style.fontSize = width/";
    s += std::to_string(5);
    s += "+\"px\";}</script><style>body {background-color: ";
    s += backgroundColor;
    s += ";}.text {color: ";
    s += fontColor;
    s += ";text-align: center;position: absolute;top: 50%;left: 50%;transform: translate(-50%, -50%);width: "
         "100%;height: 100%;display: flex;align-items: center;justify-content: center;border-radius: 0 "
         "!important;}</style></head><body><div class=\"text\">";
    s += url;
    s += "</div></body></html>";

    std::string mime_type_ = "text/html";
    auto stream = CefStreamReader::CreateForData((void*)s.c_str(), s.length());
    return new CefStreamResourceHandler(mime_type_, stream);
  } else {
    if (!app_) {
      return nullptr;
    }

    auto client = browser->GetHost()->GetClient();
    if (!client) {
      return nullptr;
    }

    auto handler = app_->GetClientHandler(client.get());
    if (!handler) {
      return nullptr;
    }
    return new CefViewSchemeHandler(browser, frame, handler);
  }
}
