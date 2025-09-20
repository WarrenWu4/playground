#include "json.h"
#include <string>

struct Settings {
};

int main() {
    Json j = Json();
    std::string path = "test.json";
    JsonValue res = j.parseFromFile(path);
    JsonObject obj = res.getObject();
    std::cout << obj.at("textColor").getString();
    return 0;
}
