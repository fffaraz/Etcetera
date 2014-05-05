#include <string>
#include <sstream>
#include <iomanip>

std::string GetHexRepresentation(const unsigned char *Bytes, size_t Length)
{
    std::ostringstream os;
    os.fill('0');
    os << std::hex;
    for(const unsigned char *ptr = Bytes; ptr < Bytes + Length; ptr++)
        os << std::setw(2) << (unsigned int)*ptr;
    return os.str();
}
