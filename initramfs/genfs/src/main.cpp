#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <print>
#include <span>
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

void pushU16LE(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back((uint8_t) (value & 0xFF));
    out.push_back((uint8_t) ((value >> 8) & 0xFF));
}

void pushU32LE(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back((uint8_t) (value & 0xFF));
    out.push_back((uint8_t) ((value >> 8) & 0xFF));
    out.push_back((uint8_t) ((value >> 16) & 0xFF));
    out.push_back((uint8_t) ((value >> 24) & 0xFF));
}
void pushPtr(std::vector<uint8_t>& out, uintptr_t value) {
    for (size_t i = 0; i < sizeof(uintptr_t); i++)
        out.push_back((uint8_t) ((value >> (i * 8)) & 0xFF));
}

struct [[gnu::packed]] DirEntry {
    // IMPORTANT: Construct dnSv before this!
    uint16_t dirHdrAm;
    uint16_t fAm;
    uint32_t dirstart;
    uint32_t fstart;
    enum class Flags : uint32_t {
        None
    } flgs;

    DirEntry(uint16_t dirHdrAm, uint16_t fAm, uint32_t dirstart, uint32_t fstart) :
        dirHdrAm(dirHdrAm), fAm(fAm), dirstart(dirstart), fstart(fstart) {}

    auto withName(std::string_view dn) -> std::vector<uint8_t> {
        std::vector<uint8_t> ret;
        pushU32LE(ret, dn.length());
        ret.insert(ret.end(), dn.begin(), dn.end());

        pushU16LE(ret, dirHdrAm);
        pushU16LE(ret, fAm);
        pushU32LE(ret, dirstart);
        pushU32LE(ret, fstart);
        pushU32LE(ret, (uint32_t) flgs);

        return ret;
    }
    auto eof() -> std::vector<uint8_t> {
        std::vector<uint8_t> ret = {};
        pushU32LE(ret, 0);
        // Conts does not exist!
        return ret;
    }
};
struct [[gnu::packed]] FileEntry { 
    // IMPORTANT: Construct fnSv before this!
    // IMPORTANT: Construct contents with sizeof(contents) == filesize
    
    FileEntry() {}

    auto genf(const std::span<const uint8_t> conts, std::string_view fn) -> std::vector<uint8_t> {
        std::vector<uint8_t> ret = {};

        pushU32LE(ret, fn.length());
        ret.insert(ret.end(), fn.begin(), fn.end());
        
        pushU32LE(ret, conts.size());
        ret.insert(ret.end(), conts.begin(), conts.end());

        return ret;
    }
    auto eof() -> std::vector<uint8_t> {
        std::vector<uint8_t> ret = {};
        pushU32LE(ret, 0);
        // No filename needed!
        pushU32LE(ret, 0); // Filesize
        // Conts not needed.

        return ret;
    }
};
struct FileData {
    std::vector<uint8_t> filedata;
    std::string fname;

    FileData(std::vector<uint8_t> filedata, std::string fname) :
        filedata(filedata), fname(fname) {}
};

struct [[gnu::packed]] SV {
    uint32_t len;
    char conts[];
};

std::vector<uint8_t> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Failed to open file");

    const auto size = std::filesystem::file_size(path);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);

    return data;
}
struct BuiltDir {
    uint32_t dirstart;
    uint32_t fstart;
    uint16_t dirHdrAm;
    uint16_t fAm;
};
uint32_t writeFileList(std::vector<uint8_t>& buf, const std::filesystem::path& dirPath, uint16_t& fAmOut) {
    uint32_t fstart = (uint32_t) buf.size();
    fAmOut = 0;
    for (auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (entry.is_directory()) continue;
        auto data = readFile(entry.path());
        std::string fname = entry.path().filename().string();
        FileEntry fe{};
        auto bytes = fe.genf(data, fname);
        buf.insert(buf.end(), bytes.begin(), bytes.end());
        fAmOut++;
    }
    FileEntry feof{};
    auto eofBytes = feof.eof();
    buf.insert(buf.end(), eofBytes.begin(), eofBytes.end());
    return fstart;
}
BuiltDir buildDir(std::vector<uint8_t>& buf, const std::filesystem::path& dirPath) {
    uint16_t fAm = 0;
    uint32_t fstart = writeFileList(buf, dirPath, fAm);

    struct Pending { std::string name; BuiltDir sub; };
    std::vector<Pending> subdirs;
    for (auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (!entry.is_directory()) continue;
        // Recurse FIRST — this is what guarantees dirstart/fstart below are real,
        // already-written offsets by the time we reference them.
        BuiltDir sub = buildDir(buf, entry.path());
        subdirs.push_back({entry.path().filename().string(), sub});
    }

    uint32_t dirstart = (uint32_t) buf.size();
    for (auto& p : subdirs) {
        DirEntry de(p.sub.dirHdrAm, p.sub.fAm, p.sub.dirstart, p.sub.fstart);
        auto bytes = de.withName(p.name);
        buf.insert(buf.end(), bytes.begin(), bytes.end());
    }
    DirEntry deof(0, 0, 0, 0);
    auto dEofBytes = deof.eof();
    buf.insert(buf.end(), dEofBytes.begin(), dEofBytes.end());

    return BuiltDir{dirstart, fstart, (uint16_t) subdirs.size(), fAm};
}

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
    std::println("Input dir absolute path: {}", inputDir.c_str());
    std::filesystem::path outputFilePath = currentShPath;
    outputFilePath = outputFilePath / flgs.getFlg(1);
    std::ofstream outputFile(outputFilePath, std::ios::binary | std::ios::trunc);
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

    std::vector<FileData> totalRootFileData = {};

    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_directory()) {
            const std::string dname = entry.path().filename().string();
            rootTotalDirNameLen += dname.length();
            std::println("/root_dir_n={}", dname);
            rootDirCount++;
        }
        else {
            std::println("File added: {}(={} bytes)", entry.path().filename().string(), entry.file_size());
            totalRootFileData.push_back(FileData(readFile(entry.path()), entry.path().filename()));
            rootFileCount++;
        }
    }
    std::println("/file_count={}", rootFileCount);
    std::println("/dir_count={}", rootDirCount);

    static_assert(sizeof(RivFsHeader) == 19, "Sizeof(RivFsHeader) != 19");
    static_assert(sizeof(FilePositionHeader) == 16, "Sizeof(FilePositionHeader) != 16");
    
    std::vector<uint8_t> buf;
    buf.resize(sizeof(RivFsHeader) + sizeof(FilePositionHeader));

    uint16_t rootFAm = 0;
    uint32_t rootFstart = writeFileList(buf, inputDir, rootFAm);

    struct Pending { std::string name; BuiltDir sub; };
    std::vector<Pending> rootSubdirs = {};
    for (auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (!entry.is_directory()) continue;
        BuiltDir sub = buildDir(buf, entry.path());
        rootSubdirs.push_back({entry.path().filename().string(), sub});
    }
    uint32_t rootDirstart = (uint32_t) buf.size();
    for (auto& p : rootSubdirs) {
        DirEntry de(p.sub.dirHdrAm, p.sub.fAm, p.sub.dirstart, p.sub.fstart);
        auto bytes = de.withName(p.name);
        buf.insert(buf.end(), bytes.begin(), bytes.end());
    }
    DirEntry rootDeof(0, 0, 0, 0);
    auto rootDEofBytes = rootDeof.eof();
    buf.insert(buf.end(), rootDEofBytes.begin(), rootDEofBytes.end());

    const std::array<uint8_t, 2> vers = {0, 0};
    RivFsHeader hdr(sizeof(FilePositionHeader), sizeof(RivFsHeader), vers, RivFsHeader::Flags::Ronly);
    FilePositionHeader fph((uint16_t) rootSubdirs.size(), rootFAm, rootDirstart, rootFstart,
        (uint32_t) FilePositionHeader::Flags::None);

    std::memcpy(buf.data(), &hdr, sizeof(RivFsHeader));

    std::memcpy(buf.data(), &hdr, sizeof(RivFsHeader));
    std::memcpy(buf.data() + sizeof(RivFsHeader), &fph, sizeof(FilePositionHeader));

    outputFile.write((const char*) buf.data(), buf.size());
}
