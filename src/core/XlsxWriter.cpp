#include "XlsxWriter.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

static uint32_t s_crcTable[256];
static bool     s_crcInit = false;

static void initCRC() {
    if (s_crcInit) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (c & 1 ? 0xEDB88320u : 0u);
        s_crcTable[i] = c;
    }
    s_crcInit = true;
}

uint32_t XlsxWriter::crc32of(const std::string& data) {
    initCRC();
    uint32_t c = 0xFFFFFFFFu;
    for (unsigned char ch : data)
        c = s_crcTable[(c ^ ch) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

void XlsxWriter::writeU16(std::vector<uint8_t>& v, uint16_t x) {
    v.push_back(x & 0xFF); v.push_back((x >> 8) & 0xFF);
}
void XlsxWriter::writeU32(std::vector<uint8_t>& v, uint32_t x) {
    v.push_back(x & 0xFF); v.push_back((x >> 8) & 0xFF);
    v.push_back((x >> 16) & 0xFF); v.push_back((x >> 24) & 0xFF);
}
void XlsxWriter::writeStr(std::vector<uint8_t>& v, const std::string& s) {
    for (unsigned char c : s) v.push_back(c);
}

void XlsxWriter::appendEntry(std::vector<uint8_t>& zip,
    std::vector<ZipEntry>& dir,
    const std::string& name,
    const std::string& data)
{
    ZipEntry e;
    e.name = name;
    e.data = data;
    e.crc32 = crc32of(data);
    e.offset = (uint32_t)zip.size();

    uint32_t sz = (uint32_t)data.size();
    uint16_t nl = (uint16_t)name.size();

    writeU32(zip, 0x04034B50u);  // signature PK\x03\x04
    writeU16(zip, 20);           // version needed
    writeU16(zip, 0);            // flags
    writeU16(zip, 0);            // method: STORED
    writeU16(zip, 0);            // mod time
    writeU16(zip, 0);            // mod date
    writeU32(zip, e.crc32);
    writeU32(zip, sz);           // compressed size
    writeU32(zip, sz);           // uncompressed size
    writeU16(zip, nl);
    writeU16(zip, 0);            // extra field length
    writeStr(zip, name);
    writeStr(zip, data);

    dir.push_back(e);
}

void XlsxWriter::addSheet(const std::string& name) {
    m_sheets.push_back({ name, {} });
}

void XlsxWriter::writeRow(const std::vector<std::string>& cells) {
    if (m_sheets.empty()) addSheet("Sheet1");
    m_sheets.back().rows.push_back(cells);
}

static std::string xmlEsc(const std::string& s) {
    std::string r; r.reserve(s.size());
    for (char c : s) {
        if (c == '&')  r += "&amp;";
        else if (c == '<')  r += "&lt;";
        else if (c == '>')  r += "&gt;";
        else if (c == '"')  r += "&quot;";
        else                r += c;
    }
    return r;
}

static std::string colName(int col) {
    std::string r;
    ++col;
    while (col > 0) {
        int rem = (col - 1) % 26;
        r = char('A' + rem) + r;
        col = (col - 1) / 26;
    }
    return r;
}

static bool looksNumeric(const std::string& s) {
    if (s.empty()) return false;
    bool hasDot = false, hasE = false;
    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') start = 1;
    if (start == s.size()) return false;
    for (size_t i = start; i < s.size(); ++i) {
        char c = s[i];
        if (c == '.') { if (hasDot) return false; hasDot = true; }
        else if (c == 'e' || c == 'E') {
            if (hasE || i == start) return false;
            hasE = true;
            if (i + 1 < s.size() && (s[i + 1] == '+' || s[i + 1] == '-')) ++i;
        }
        else if (c < '0' || c > '9') return false;
    }
    return true;
}

std::string XlsxWriter::buildContentTypes() const {
    std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\"  ContentType=\"application/xml\"/>"
        "<Override PartName=\"/xl/workbook.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
        "<Override PartName=\"/xl/styles.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
        "<Override PartName=\"/xl/sharedStrings.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>";
    for (int i = 0; i < (int)m_sheets.size(); ++i) {
        s += "<Override PartName=\"/xl/worksheets/sheet" + std::to_string(i + 1) + ".xml\""
            " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
    }
    s += "</Types>";
    return s;
}

std::string XlsxWriter::buildRels() const {
    return
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\""
        " Target=\"xl/workbook.xml\"/>"
        "</Relationships>";
}

std::string XlsxWriter::buildWorkbook() const {
    std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheets>";
    for (int i = 0; i < (int)m_sheets.size(); ++i) {
        s += "<sheet name=\"" + xmlEsc(m_sheets[i].name) + "\""
            " sheetId=\"" + std::to_string(i + 1) + "\""
            " r:id=\"rId" + std::to_string(i + 1) + "\"/>";
    }
    s += "</sheets></workbook>";
    return s;
}

std::string XlsxWriter::buildWorkbookRels() const {
    std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    for (int i = 0; i < (int)m_sheets.size(); ++i) {
        s += "<Relationship Id=\"rId" + std::to_string(i + 1) + "\""
            " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\""
            " Target=\"worksheets/sheet" + std::to_string(i + 1) + ".xml\"/>";
    }
    int n = (int)m_sheets.size();
    s += "<Relationship Id=\"rId" + std::to_string(n + 1) + "\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\""
        " Target=\"sharedStrings.xml\"/>"
        "<Relationship Id=\"rId" + std::to_string(n + 2) + "\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\""
        " Target=\"styles.xml\"/>"
        "</Relationships>";
    return s;
}

std::string XlsxWriter::buildStyles() const {
    return
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<fonts count=\"2\">"
        "<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
        "<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font>"
        "</fonts>"
        "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill>"
        "<fill><patternFill patternType=\"gray125\"/></fill></fills>"
        "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
        "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
        "<cellXfs count=\"2\">"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
        "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
        "</cellXfs>"
        "</styleSheet>";
}

std::string XlsxWriter::buildSharedStrings(std::vector<std::string>& outTable) const {
    outTable.clear();
    int total = 0;
    for (const auto& sh : m_sheets)
        for (const auto& row : sh.rows)
            for (const auto& cell : row) {
                if (!looksNumeric(cell)) {
                    if (std::find(outTable.begin(), outTable.end(), cell) == outTable.end())
                        outTable.push_back(cell);
                    ++total;
                }
            }

    std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\""
        " count=\"" + std::to_string(total) + "\""
        " uniqueCount=\"" + std::to_string(outTable.size()) + "\">";
    for (const auto& str : outTable)
        s += "<si><t>" + xmlEsc(str) + "</t></si>";
    s += "</sst>";
    return s;
}

std::string XlsxWriter::buildSheet(const Sheet& sh,
    const std::vector<std::string>& strTable) const
{
    std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<sheetData>";

    for (int ri = 0; ri < (int)sh.rows.size(); ++ri) {
        const auto& row = sh.rows[ri];
        s += "<row r=\"" + std::to_string(ri + 1) + "\">";
        bool isHeader = (ri == 0);
        for (int ci = 0; ci < (int)row.size(); ++ci) {
            const std::string& cell = row[ci];
            std::string ref = colName(ci) + std::to_string(ri + 1);
            if (looksNumeric(cell)) {
                s += "<c r=\"" + ref + "\"";
                if (isHeader) s += " s=\"1\"";
                s += "><v>" + cell + "</v></c>";
            }
            else {
                auto it = std::find(strTable.begin(), strTable.end(), cell);
                int idx = (it != strTable.end()) ? (int)(it - strTable.begin()) : 0;
                s += "<c r=\"" + ref + "\" t=\"s\"";
                if (isHeader) s += " s=\"1\"";
                s += "><v>" + std::to_string(idx) + "</v></c>";
            }
        }
        s += "</row>";
    }
    s += "</sheetData></worksheet>";
    return s;
}

bool XlsxWriter::save(const std::string& path) const {
    if (m_sheets.empty()) return false;

    std::vector<uint8_t>  zip;
    std::vector<ZipEntry> dir;

    std::vector<std::string> strTable;
    std::string sharedStr = buildSharedStrings(strTable);

    appendEntry(zip, dir, "[Content_Types].xml", buildContentTypes());
    appendEntry(zip, dir, "_rels/.rels", buildRels());
    appendEntry(zip, dir, "xl/workbook.xml", buildWorkbook());
    appendEntry(zip, dir, "xl/_rels/workbook.xml.rels", buildWorkbookRels());
    appendEntry(zip, dir, "xl/styles.xml", buildStyles());
    appendEntry(zip, dir, "xl/sharedStrings.xml", sharedStr);

    for (int i = 0; i < (int)m_sheets.size(); ++i) {
        std::string sheetXml = buildSheet(m_sheets[i], strTable);
        appendEntry(zip, dir,
            "xl/worksheets/sheet" + std::to_string(i + 1) + ".xml",
            sheetXml);
    }

    uint32_t cdOffset = (uint32_t)zip.size();
    for (const auto& e : dir) {
        uint32_t sz = (uint32_t)e.data.size();
        uint16_t nl = (uint16_t)e.name.size();
        writeU32(zip, 0x02014B50u);
        writeU16(zip, 20); 
        writeU16(zip, 20);
        writeU16(zip, 0);
        writeU16(zip, 0);
        writeU16(zip, 0);
        writeU16(zip, 0);
        writeU32(zip, e.crc32);
        writeU32(zip, sz);
        writeU32(zip, sz);
        writeU16(zip, nl);
        writeU16(zip, 0);  writeU16(zip, 0); 
        writeU16(zip, 0);  writeU16(zip, 0); 
        writeU32(zip, 0);                    
        writeU32(zip, e.offset);
        writeStr(zip, e.name);
    }

    uint32_t cdSize = (uint32_t)zip.size() - cdOffset;

    writeU32(zip, 0x06054B50u);
    writeU16(zip, 0); writeU16(zip, 0);
    writeU16(zip, (uint16_t)dir.size());
    writeU16(zip, (uint16_t)dir.size());
    writeU32(zip, cdSize);
    writeU32(zip, cdOffset);
    writeU16(zip, 0);

    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(reinterpret_cast<const char*>(zip.data()), (std::streamsize)zip.size());
    return f.good();
}