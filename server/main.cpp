#include "crow.h"

static const std::string SAVE_FILE_NAME = "uploaded_save.zip";
static const std::string SAVE_FILE_EXTENSION = ".zip";

inline static std::filesystem::path get_save_file_path() {
#if defined(_WIN32)
    const char* programData = std::getenv("PROGRAMDATA");
    if (programData && programData[0] != '\0') {
        return std::filesystem::path(programData) / "MineSync" / "uploads";
    }
    return std::filesystem::path("C:/ProgramData/MineSync/uploads");
#else
    return ("/var/lib/minesync/uploads");
#endif
}

int main() {
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

        std::string filename = std::filesystem::path(name).filename().string();
        std::filesystem::path uploadDir = get_save_file_path();
        std::error_code ec;
        std::filesystem::create_directory(uploadDir, ec);
        std::filesystem::path targetPath = uploadDir / filename;

        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            return crow::response(500, "Error: Failed to open file for writing on server.");
        }

        outFile.write(fileBody.data(), fileBody.size());
        outFile.close();

        crow::json::wvalue responseJson;
        responseJson["status"] = "success";

        return crow::response(200, responseJson);
    });

    app.port(18080).multithreaded().run();
}