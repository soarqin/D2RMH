#ifndef NODEUV_URI
#define NODEUV_URI

#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <vector>
#include <sstream>

namespace uri {

using namespace std;

typedef map<string, string> qsmap;
typedef vector<string> split;

struct url {
    string protocol;
    string user;
    string password;
    string host;
    string path;
    string search;
    qsmap query;
    int port = 0;
};

//--- Helper Functions -------------------------------------------------------------~
static inline string TailSlice(string &subject, const string &delimiter, bool keep_delim = false) {
    // Chops off the delimiter and everything that follows (destructively)
    // returns everything after the delimiter
    auto delimiter_location = subject.find(delimiter);
    auto delimiter_length = delimiter.length();
    string output;

    if (delimiter_location < string::npos) {
        auto start = keep_delim ? delimiter_location : delimiter_location + delimiter_length;
        auto end = subject.length() - start;
        output = subject.substr(start, end);
        subject = subject.substr(0, delimiter_location);
    }
    return output;
}

static inline string HeadSlice(string &subject, const string &delimiter) {
    // Chops off the delimiter and everything that precedes (destructively)
    // returns everthing before the delimeter
    auto delimiter_location = subject.find(delimiter);
    auto delimiter_length = delimiter.length();
    string output;
    if (delimiter_location < string::npos) {
        output = subject.substr(0, delimiter_location);
        subject = subject.substr(
            delimiter_location + delimiter_length,
            subject.length() - (delimiter_location + delimiter_length));
    }
    return output;
}

static inline split Split(const std::string &input, const string &separators, bool remove_empty = true) {
    split list;
    ostringstream word;
    for (char n : input) {
        if (string::npos == separators.find(n))
            word << n;
        else {
            if (!word.str().empty() || !remove_empty) {
                list.push_back(word.str());
            }
            word.str("");
        }
    }
    if (!word.str().empty() || !remove_empty) {
        list.push_back(word.str());
    }
    return list;
}

//--- Extractors -------------------------------------------------------------------~
static inline int ExtractPort(string &hostport) {
    int port;
    string portstring = TailSlice(hostport, ":");
    try { port = int(strtol(portstring.c_str(), nullptr, 0)); }
    catch (const exception &e) { port = -1; }
    return port;
}

static inline string ExtractPath(string &in) { return TailSlice(in, "/", true); }
static inline string ExtractProtocol(string &in) { return HeadSlice(in, "://"); }
static inline string ExtractSearch(string &in) { return TailSlice(in, "?"); }
static inline string ExtractPassword(string &userpass) { return TailSlice(userpass, ":"); }
static inline string ExtractUserpass(string &in) { return HeadSlice(in, "@"); }

//--- Public Interface -------------------------------------------------------------~
static inline url ParseHttpUrl(string &in) {

    url ret;
    ret.port = -1;

    ret.search = ExtractSearch(in);
    ret.protocol = ExtractProtocol(in);

    ret.path = ExtractPath(in);

    string userpass = ExtractUserpass(in);
    ret.password = ExtractPassword(userpass);
    ret.user = userpass;

    ret.port = ExtractPort(in);
    ret.host = in;

    auto pairs = Split(ret.search, "&");

    for (auto &p: pairs) {
        auto pair = Split(p, "=");
        if (pair.size() == 2) {
            ret.query.insert({pair[0], pair[1]});
        }
    }
    return ret;
}

const char HEX2DEC[256] = {
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 3 */  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

    /* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

    /* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

    /* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static inline std::string UriDecode(const std::string &sSrc) {

    // Note from RFC1630:  "Sequences which start with a percent sign
    // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
    // for future extension"

    const auto *pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    const unsigned char *const SRC_END = pSrc + SRC_LEN;
    const unsigned char *const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%'

    char *const pStart = new char[SRC_LEN];
    char *pEnd = pStart;

    while (pSrc < SRC_LAST_DEC) {
        if (*pSrc == '%') {
            char dec1, dec2;
            if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                && -1 != (dec2 = HEX2DEC[*(pSrc + 2)])) {

                *pEnd++ = (dec1 << 4) + dec2;
                pSrc += 3;
                continue;
            }
        }
        *pEnd++ = *pSrc++;
    }

    // the last 2- chars
    while (pSrc < SRC_END) {
        *pEnd++ = *pSrc++;
    }

    std::string sResult(pStart, pEnd);
    delete[] pStart;
    return sResult;
}

// Only alphanum is safe.
const char SAFE[256] = {
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

    /* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    /* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,

    /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    /* C */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* D */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* E */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static inline std::string UriEncode(const std::string &sSrc) {
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const auto *pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    auto *const pStart = new unsigned char[SRC_LEN * 3];
    auto *pEnd = pStart;
    const auto *const SRC_END = pSrc + SRC_LEN;

    for (; pSrc < SRC_END; ++pSrc) {
        if (SAFE[*pSrc]) {
            *pEnd++ = *pSrc;
        } else {
            // escape this char
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
    }

    std::string sResult((char *)pStart, (char *)pEnd);
    delete[] pStart;
    return sResult;
}
}

#endif
