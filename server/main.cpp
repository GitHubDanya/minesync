#include "crow.h"
#include "lib/CLI11.hpp"

static const int SERVER_PORT = 18080;

static const std::string SAVE_FILE_NAME = "uploaded_save.zip";
static const std::string SAVE_FILE_EXTENSION = ".zip";

int main(int argc, char** argv) {
	CLI::App cliApp{"Minesync"};
    argv = cliApp.ensure_utf8(argv);

    int serverPort = SERVER_PORT;
    cliApp.add_option("-p,--port", serverPort, "Port for the server");

    CLI11_PARSE(cliApp, argc, argv);

	crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    .methods(crow::HTTPMethod::GET)([]() {
        return "Hello world!\n";
    });

    CROW_ROUTE(app, "/sync")
    .methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        const char* filename = req.url_params.get("name");
        if (!filename)
            return crow::response(400, "Missing 'name' query parameter");

        std::ifstream file(( "./" + std::string(filename) ), std::ios::binary);
        if (!file.is_open()) {
            return crow::response(500, "Error: Failed to open file for writing on server.");
        }

        std::ostringstream contents;
        contents << file.rdbuf();
        file.close();

        crow::response res;
        res.code = 200;
        res.body = contents.str();

        res.set_header("Content-Type", "application/octet-stream");
        res.set_header("Content-Disposition",  "attachment; filename=\"" + std::string(filename) + "\"");

        return res;
    });

    CROW_ROUTE(app, "/upload")
    .methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        crow::multipart::message multipartMsg(req);

        auto [nameHeaders, name] = multipartMsg.get_part_by_name("name");
        if (name.empty())
            return crow::response(400, "Error: Invalid or missing name key.");

        auto [fileHeaders, fileBody] = multipartMsg.get_part_by_name("file");
        if (fileBody.empty())
            return crow::response(400, "Error: No file uploaded or 'file' key missing.");

        std::string filename = name;
        auto headers = fileHeaders;

        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            return crow::response(500, "Error: Failed to open file for writing on server.");
        }

        outFile << fileBody;
        outFile.close();

        crow::json::wvalue responseJson;
        responseJson["status"] = "success";

        return crow::response(200, responseJson);
    });
    app.port(serverPort).multithreaded().run();
}