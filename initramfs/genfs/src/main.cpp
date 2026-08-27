#include <array>
#include <cstdint>
#include <format>
#include <print>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <fstream>
#include <filesystem>

struct InputFlags {
private:
    struct InputFlag {
        std::string conts; 
        InputFlag(std::string iconts) :
            conts(iconts) {}
    };

    std::vector<InputFlag> flgs = {};

public:
    auto fromArgv(int argc, char** argv) {
        // Skip first argument (path-to-exec)
        argv++;
        argc--;

        for (int i = 0; i < argc; i++) {
            flgs.push_back(InputFlag(*argv));
            argv++;
        }
    }
    auto containsFlg(std::string_view flg) -> bool {
        for (uint32_t i = 0; i < flgs.size(); i++) {
            if (flgs[i].conts == flg) return true;
        } 

        return false;
    }
    auto getArgCount() -> uint32_t {
        return flgs.size();
    }
    auto getFlg(uint32_t i) -> std::string {
        if (i >= flgs.size()) {
            throw std::runtime_error(std::format("OOB flag: i={}", i));
        }
        return flgs[i].conts;
    }
};

struct [[gnu::packed]] RivFsHeader {
private:
    static constexpr std::array<char, 5> MAGIC = {'r', 'i', 'v', 'f', 's'};
public:
    char magic[5];
    uint32_t fphSize;
    uint32_t fphPos;
    uint8_t vers[2];
    uint8_t endingZeroer;
    enum class Flags : uint32_t {
        None,
        Ronly, // Should be ON!
    } flgs;
    RivFsHeader(uint32_t ifphSize, uint32_t ifphPos, std::array<uint8_t, 2> ivers, Flags iflgs) :
    fphSize(ifphSize), fphPos(ifphPos), flgs(iflgs), endingZeroer(0) {
        std::copy(ivers.begin(), ivers.end(), vers);
        std::copy(MAGIC.begin(), MAGIC.end(), magic);
    }
};

int main(int argc, char* argv[]) {
    InputFlags flgs;
    flgs.fromArgv(argc, argv);

    if (flgs.getArgCount() < 2) {
        std::println("Usage: ./path/to/exec input_dir/ out_img\n");
        return 1;
    } 
    const std::string currentShPath = std::filesystem::current_path();
    std::filesystem::path inputDir(flgs.getFlg(0));

    if (inputDir.is_relative()) {
        inputDir = std::filesystem::path(currentShPath) / inputDir;
    }
    std::println("Input file absolute path: {}", inputDir.c_str());
    std::filesystem::path outputFilePath = currentShPath;
    outputFilePath = outputFilePath / flgs.getFlg(1);
    std::ofstream outputFile(outputFilePath);
    std::println("Output file(path)={}", outputFilePath.string());
    if (!outputFile) {
        std::println("Opening output file failed\nNOTE: This is not caused by the file not existing");
        return 1;
    }

    if (!std::filesystem::exists(inputDir)) {
        std::println("Directory did not exist: {}", inputDir.c_str());
        if (inputDir != std::filesystem::path(flgs.getFlg(0))) {
            std::println("NOTE: This seems to be a relative path (path={})", flgs.getFlg(0));
        }
        return 1;
    }
    
    // RivFsHeader(uint32_t ifphSize, uint32_t ifphPos, std::array<uint8_t, 2> ivers, Flags iflgs)
    RivFsHeader hdr(0, 0, {0, 0}, RivFsHeader::Flags::Ronly);
    // Now we can construct the rivfs.
    const char* hdrCStr = (const char*) (&hdr); // Fuck strict aliasing!
    outputFile.write(hdrCStr, sizeof(RivFsHeader));

    return 0;
}
