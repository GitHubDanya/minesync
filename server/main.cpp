#include "crow.h"

static int SERVER_PORT = 18080;

static const std::string SAVE_FILE_NAME = "uploaded_save.zip";
static const std::string SAVE_FILE_EXTENSION = ".zip";

int main() {
	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "--help" || arg == "-h")
		{
			std::cout << "Usage: minesync_server.exe [options] <arguments>\n"
				<< "options:\n"
				<< "  -h, --help\t\tDisplay this help message\n"
				<< "  -P, --port\t\tSpecify server Port number\n\n";
			return 0;
		}
		if (i + 1 <= argc)
		{
			try
			{
				if (arg == "--port" || arg == "-P")
				{
					SERVER_PORT = std::stoi(argv[i + 1]);
				}
			}
			catch (std::invalid_argument& e)
			{
				std::cerr << "Error:port must be a valid number\n";
				return 1;
			}
		}
		else
		{
			std::cerr << "Error: port requires a value\n";
			return 1;
		}
	}

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

    app.port(SERVER_PORT).multithreaded().run();
}