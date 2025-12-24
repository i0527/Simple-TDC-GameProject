#include <raylib.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// PNGファイルのサイズを読み取る
bool GetImageSize(const std::string& imagePath, int& width, int& height) {
    Image img = LoadImage(imagePath.c_str());
    if (img.data == nullptr) {
        return false;
    }
    width = img.width;
    height = img.height;
    UnloadImage(img);
    return true;
}

// アクション名からJSONファイル名を生成
std::string GetJsonFileName(const std::string& action) {
    if (action == "idle") return "idle.json";
    if (action == "walk") return "walk.json";
    if (action == "attack") return "attack.json";
    if (action == "death") return "die.json";
    return "";
}

// PNGファイルからJSONファイルを生成
bool GenerateJsonFromPng(const std::string& pngPath, const std::string& jsonPath, const std::string& actionName) {
    int width = 0, height = 0;
    if (!GetImageSize(pngPath, width, height)) {
        std::cerr << "Failed to load image: " << pngPath << std::endl;
        return false;
    }
    
    if (height <= 0) {
        std::cerr << "Invalid image height: " << pngPath << std::endl;
        return false;
    }
    
    // フレーム数を計算（横サイズを縦サイズで割る）
    int frameCount = width / height;
    if (frameCount <= 0) {
        std::cerr << "Invalid frame count: " << frameCount << " for " << pngPath << std::endl;
        return false;
    }
    
    // JSONファイルを生成
    nlohmann::json j;
    j["frames"] = nlohmann::json::object();
    
    // 各フレームを生成
    for (int i = 0; i < frameCount; ++i) {
        std::string frameName = actionName + "-" + std::to_string(i) + ".aseprite";
        nlohmann::json frame;
        frame["frame"]["x"] = i * height;
        frame["frame"]["y"] = 0;
        frame["frame"]["w"] = height;
        frame["frame"]["h"] = height;
        frame["rotated"] = false;
        frame["trimmed"] = false;
        frame["spriteSourceSize"]["x"] = 0;
        frame["spriteSourceSize"]["y"] = 0;
        frame["spriteSourceSize"]["w"] = height;
        frame["spriteSourceSize"]["h"] = height;
        frame["sourceSize"]["w"] = height;
        frame["sourceSize"]["h"] = height;
        frame["duration"] = 100;
        
        j["frames"][frameName] = frame;
    }
    
    // meta情報を生成
    j["meta"]["app"] = "https://www.aseprite.org/";
    j["meta"]["version"] = "1.3.x";
    j["meta"]["image"] = fs::path(pngPath).filename().string();
    j["meta"]["format"] = "RGBA8888";
    j["meta"]["size"]["w"] = width;
    j["meta"]["size"]["h"] = height;
    j["meta"]["scale"] = "1";
    
    // frameTagsを生成
    nlohmann::json frameTag;
    frameTag["name"] = actionName;
    frameTag["from"] = 0;
    frameTag["to"] = frameCount - 1;
    frameTag["direction"] = "forward";
    j["meta"]["frameTags"] = nlohmann::json::array({frameTag});
    
    // JSONファイルを書き込み
    std::ofstream out(jsonPath);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << jsonPath << std::endl;
        return false;
    }
    
    out << j.dump(2) << std::endl;
    out.close();
    
    std::cout << "Generated: " << jsonPath << " (frames: " << frameCount << ")" << std::endl;
    return true;
}

// キャラクターフォルダからJSONファイルを生成
void GenerateCharacterJsonFiles(const std::string& characterFolder) {
    std::cout << "Processing: " << characterFolder << std::endl;
    
    const std::vector<std::pair<std::string, std::string>> actions = {
        {"idle", "idle.png"},
        {"walk", "walk.png"},
        {"attack", "attack.png"},
        {"death", "die.png"}
    };
    
    for (const auto& [action, pngFile] : actions) {
        fs::path pngPath = fs::path(characterFolder) / pngFile;
        if (!fs::exists(pngPath)) {
            std::cout << "  Skipping " << pngFile << " (not found)" << std::endl;
            continue;
        }
        
        fs::path jsonPath = fs::path(characterFolder) / GetJsonFileName(action);
        if (!GenerateJsonFromPng(pngPath.string(), jsonPath.string(), action)) {
            std::cerr << "  Failed to generate JSON for " << pngFile << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // Raylib初期化（画像読み込み用）
    InitWindow(1, 1, "JSON Generator");
    
    std::string basePath = "assets/characters";
    
    // subフォルダのキャラクターを処理
    std::vector<std::string> characters = {
        "sub/HatSlime",
        "sub/LanterfishAnglerfish",
        "sub/LongTailedTit",
        "sub/Orca",
        "sub/Rainbow",
        "sub/SeaHorse",
        "sub/Whale",
        "sub/YodarehakiDragonfish"
    };
    
    for (const auto& character : characters) {
        fs::path characterPath = fs::path(basePath) / character;
        if (fs::exists(characterPath) && fs::is_directory(characterPath)) {
            GenerateCharacterJsonFiles(characterPath.string());
        } else {
            std::cerr << "Character folder not found: " << characterPath << std::endl;
        }
    }
    
    CloseWindow();
    return 0;
}

