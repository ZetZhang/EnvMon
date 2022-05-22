#include <drogon/drogon.h>

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0",12306);
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop

    drogon::HttpAppFramework::instance()
        .registerHandler("/list_para", // 把浏览器发来的HTTP请求参数显示在返回的html页面里
                [=](const drogon::HttpRequestPtr &req,
                    std::function<void (const drogon::HttpResponsePtr &)> &&callback)
        {
            auto para = req->getParameters();
            drogon::HttpViewData data;
            data.insert("title", "ListParameters");
            data.insert("parameters", para);
            auto resp = drogon::HttpResponse::newHttpViewResponse("ListParameters.csp", data);
            callback(resp);
        })
        .registerHandler("/slow", // session的使用
                [=](const drogon::HttpRequestPtr &req,
                    std::function<void (const drogon::HttpResponsePtr &)> &&callback)
                {
                    Json::Value json;
                    json["result"] = "ok";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
                    callback(resp);
                }, { drogon::Get, "TimeFilter" })
        .enableSession(1200);
    // 在views文件夹下创建视图文件ListParameters.csp，然后使用命令
    // drogon_ctl create view ListParamters.csp将csp文件转换为C++源文件

    drogon::app().run();
    return 0;
}
