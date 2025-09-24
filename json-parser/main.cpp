#include "json.h"
#include <string>

struct Settings {
};

int main() {
    Json j = Json();
    std::string path = "test.json";
    JsonValue res = j.parseFromFile(path);
    std::string jsonStr = j.stringify(res);
    j.writeToFile("res.json", jsonStr);
    return 0;
}
