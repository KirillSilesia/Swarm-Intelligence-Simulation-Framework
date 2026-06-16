#pragma once
/*
 * Minimal XLSX writer — no external dependencies.
 * Produces a valid Office Open XML workbook (.xlsx) using STORED (uncompressed)
 * ZIP entries, which every version of Excel, LibreOffice, and Google Sheets
 * supports.
 *
 * Usage:
 *   XlsxWriter xls;
 *   xls.addSheet("Summary");
 *   xls.writeRow({"Algorithm","Time","Score"});
 *   xls.writeRow({"PSO", "12.3", "0.95"});
 *   xls.addSheet("Convergence");
 *   xls.writeRow({"Time","BestFitness"});
 *   xls.writeRow({"0.25","1.23"});
 *   xls.save("report.xlsx");
 */
#include <string>
#include <vector>
#include <cstdint>

class XlsxWriter {
public:
    // Start a new sheet (must call before writeRow for that sheet)
    void addSheet(const std::string& name);

    // Append one row to the current (most recently added) sheet
    void writeRow(const std::vector<std::string>& cells);

    // Write the .xlsx file to disk.  Returns true on success.
    bool save(const std::string& path) const;

private:
    struct Sheet {
        std::string name;
        std::vector<std::vector<std::string>> rows;
    };
    std::vector<Sheet> m_sheets;

    // ---- ZIP helpers ----
    struct ZipEntry {
        std::string name;
        std::string data;
        uint32_t    crc32;
        uint32_t    offset;
    };

    static uint32_t crc32of(const std::string& data);

    // Build the XML strings
    std::string buildContentTypes() const;
    std::string buildRels()         const;
    std::string buildWorkbook()     const;
    std::string buildWorkbookRels() const;
    std::string buildStyles()       const;
    std::string buildSharedStrings(std::vector<std::string>& outTable) const;
    std::string buildSheet(const Sheet& s,
        const std::vector<std::string>& strTable) const;

    // ZIP serialisation
    static void writeU16(std::vector<uint8_t>& v, uint16_t x);
    static void writeU32(std::vector<uint8_t>& v, uint32_t x);
    static void writeStr(std::vector<uint8_t>& v, const std::string& s);
    static void appendEntry(std::vector<uint8_t>& zip,
        std::vector<ZipEntry>& dir,
        const std::string& name,
        const std::string& data);
};