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
    enum class Flags : uint32_t {
        None,
        Ronly, // Should be ON!
    } flgs;
    RivFsHeader(uint32_t ifphSize, uint32_t ifphPos, std::array<uint8_t, 2> ivers, Flags iflgs) :
    fphSize(ifphSize), fphPos(ifphPos), flgs(iflgs) {
        std::copy(ivers.begin(), ivers.end(), vers);
        std::copy(MAGIC.begin(), MAGIC.end(), magic);
    }
};

struct [[gnu::packed]] FilePositionHeader {
    uint16_t dirHdrAm;
    uint16_t fAm;
    uint32_t dirstart;
    uint32_t fstart;
    enum class Flags : uint32_t {
        None
    } flgs;

    FilePositionHeader(uint16_t dirHdrAm, uint16_t fAm, uint32_t dirstart, uint32_t fstart, uint32_t flgs) :
        dirHdrAm(dirHdrAm), fAm(fAm), dirstart(dirstart), fstart(fstart), flgs((Flags) flgs) {}
};

struct [[gnu::packed]] DirEntry {
    // IMPORTANT: Construct dnSv before this!
    uint16_t dirHdrAm;
    uint16_t fAm;
    uint32_t dirstart;
    uint32_t fstart;
    enum class Flags : uint32_t {
        None
    } flgs;
};
struct [[gnu::packed]] FileEntry { 
    // IMPORTANT: Construct fnSv before this!
    uint32_t filesize;
    char conts[]; // Must == filesize!
};

struct [[gnu::packed]] SV {
    uint32_t len;
    char conts[];
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
    uint16_t rootFileCount = 0;
    uint16_t rootDirCount =  0;

    uint32_t rootTotalDirNameLen = 0;

    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_directory()) {
            const std::string dname = entry.path().filename().string();
            rootTotalDirNameLen += dname.length();
            std::println("/root_dir_n={}", dname);
            rootDirCount++;
        }
        else {
            rootFileCount++;
        }
    }
    std::println("/file_count={}", rootFileCount);
    std::println("/dir_count={}", rootDirCount);

    static_assert(sizeof(RivFsHeader) == 19, "Sizeof(RivFsHeader) != 19");
    static_assert(sizeof(FilePositionHeader) == 16, "Sizeof(FilePositionHeader) != 16");
    
    // RivFsHeader(uint32_t ifphSize, uint32_t ifphPos, std::array<uint8_t, 2> ivers, Flags iflgs)
    // FilePositionHeader(uint16_t dirHdrAm, uint16_t fAm, uint32_t dirstart, uint32_t fstart, uint32_t flgs)
    
    const auto fphPos = sizeof(RivFsHeader);
    const std::array<uint8_t, 2> vers = {0, 0}; 
    std::println("rivfs version: {}.{}", vers[0], vers[1]);
    RivFsHeader hdr(sizeof(FilePositionHeader), fphPos, vers, RivFsHeader::Flags::Ronly);

    const uint32_t dirStart = hdr.fphPos + sizeof(FilePositionHeader);
    const uint32_t fileStart = rootTotalDirNameLen + dirStart + (sizeof(SV::len) + sizeof(DirEntry) * rootDirCount);
    std::println("/dir_start={}", dirStart);
    std::println("/file_start={}", fileStart);

    FilePositionHeader fph(rootDirCount, rootFileCount, dirStart, fileStart, (uint32_t) FilePositionHeader::Flags::None);
    // Now we can construct the rivfs.
    outputFile.write((const char*) &hdr, sizeof(RivFsHeader));
    outputFile.write((const char*) &fph, sizeof(FilePositionHeader));
}
