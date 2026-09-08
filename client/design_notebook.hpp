#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
namespace design_notebook {
// Independent files survive new journeys and can be handed directly to the next iteration.
inline std::vector<std::filesystem::path> entries(const std::filesystem::path &dir) {
    std::vector<std::filesystem::path> out;
    std::error_code ec;
    for (std::filesystem::directory_iterator i(dir, ec), end; !ec && i != end; i.increment(ec))
        if (i->is_regular_file(ec) && i->path().extension() == ".md")
            out.push_back(i->path());
    std::sort(out.rbegin(), out.rend());
    return out;
}
inline std::string read(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    std::string s(32768, '\0');
    in.read(s.data(), s.size());
    s.resize(size_t(in.gcount()));
    return s;
}
inline bool save(const std::filesystem::path &dir, const std::string &context,
                 const std::string &body, const std::vector<uint8_t> &snapshot,
                 std::string &error) {
    if (body.find_first_not_of(" \t\r\n") == std::string::npos) {
        error = "Write a note first.";
        return false;
    }
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    auto id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    auto path = dir / (id + ".md");
    for (int n = 0; std::filesystem::exists(path); ++n)
        path = dir / (id + "-" + std::to_string(n) + ".md");
    auto tmp = path;
    tmp += ".tmp";
    auto replay = path;
    replay.replace_extension(".crlb");
    if (!snapshot.empty()) {
        std::ofstream f(replay, std::ios::binary);
        if (!f.write(reinterpret_cast<const char *>(snapshot.data()), snapshot.size()) ||
            !f.flush()) {
            error = "Could not save combat snapshot. Draft kept.";
            return false;
        }
    }
    {
        std::ofstream f(tmp, std::ios::binary);
        f << "# Tinikami design note\n\n" << context << "\n## Feedback\n\n" << body << "\n";
        if (!snapshot.empty())
            f << "\nCombat snapshot: " << replay.filename().string() << "\n";
        if (!f.flush()) {
            error = "Could not write note. Draft kept.";
            return false;
        }
    }
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    error.clear();
    return true;
}
struct Line {
    std::string text;
    size_t begin, end;
    std::vector<size_t> offsets;
};
inline size_t previous(const std::string &s, size_t pos) {
    if (!pos)
        return 0;
    --pos;
    while (pos && (static_cast<unsigned char>(s[pos]) & 0xc0) == 0x80)
        --pos;
    return pos;
}
inline size_t next(const std::string &s, size_t pos) {
    if (pos >= s.size())
        return s.size();
    ++pos;
    while (pos < s.size() && (static_cast<unsigned char>(s[pos]) & 0xc0) == 0x80)
        ++pos;
    return pos;
}
inline std::vector<Line> layout(const std::string &s, size_t width) {
    std::vector<Line> out;
    size_t start = 0;
    do {
        size_t end = start, space = std::string::npos, columns = 0;
        while (end < s.size() && s[end] != '\n' && columns < width) {
            if (s[end] == ' ')
                space = end;
            end = next(s, end);
            ++columns;
        }
        if (end < s.size() && s[end] != '\n' && space != std::string::npos && space > start)
            end = space + 1;
        Line line{"", start, end, {}};
        for (size_t at = start; at < end; at = next(s, at)) {
            line.offsets.push_back(at);
            unsigned char c = s[at];
            line.text += c == '\t' ? ' ' : c < 128 ? char(c) : '?';
        }
        line.offsets.push_back(end);
        out.push_back(line);
        if (end >= s.size())
            break;
        start = end + (s[end] == '\n');
    } while (start <= s.size());
    return out;
}
} // namespace design_notebook
