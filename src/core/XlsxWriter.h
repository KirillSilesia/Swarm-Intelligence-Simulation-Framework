#pragma once
#include <string>
#include <vector>
#include <cstdint>

class XlsxWriter {
public:
    void addSheet(const std::string& name);

    void writeRow(const std::vector<std::string>& cells);

    bool save(const std::string& path) const;

private:
    struct Sheet {
        std::string name;
        std::vector<std::vector<std::string>> rows;
    };
    std::vector<Sheet> m_sheets;

    struct ZipEntry {
        std::string name;
        std::string data;
        uint32_t    crc32;
        uint32_t    offset;
    };

    static uint32_t crc32of(const std::string& data);

    std::string buildContentTypes() const;
    std::string buildRels()         const;
    std::string buildWorkbook()     const;
    std::string buildWorkbookRels() const;
    std::string buildStyles()       const;
    std::string buildSharedStrings(std::vector<std::string>& outTable) const;
    std::string buildSheet(const Sheet& s,
        const std::vector<std::string>& strTable) const;

    static void writeU16(std::vector<uint8_t>& v, uint16_t x);
    static void writeU32(std::vector<uint8_t>& v, uint32_t x);
    static void writeStr(std::vector<uint8_t>& v, const std::string& s);
    static void appendEntry(std::vector<uint8_t>& zip,
        std::vector<ZipEntry>& dir,
        const std::string& name,
        const std::string& data);
};