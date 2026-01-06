#ifndef FTP_LISTING_PARSER_H
#define FTP_LISTING_PARSER_H

#include <QString>
#include <QStringList>

/**
 * @brief Parser for FTP directory listings
 * 
 * Parses Unix-style FTP directory listings (from LIST command)
 * Format: drwxr-xr-x 2 user group 4096 Jan 1 12:00 filename
 */
class FTPListingParser
{
public:
    /**
     * @brief Parse FTP directory listing string into a list of items
     * 
     * Each item is formatted as: "filename\tsize\tdate"
     * - filename: The file or directory name
     * - size: File size in bytes, or "DIR" for directories
     * - date: Modification date (e.g., "Jan 1 12:00")
     * 
     * @param listing Raw listing string from FTP server
     * @return List of parsed items, empty list on error
     */
    static QStringList parseListing(const QString &listing);

    /**
     * @brief Parse FTP directory listing string into a list of items
     * 
     * Overload for std::string input
     * 
     * @param listing Raw listing string from FTP server
     * @return List of parsed items, empty list on error
     */
    static QStringList parseListing(const std::string &listing);

private:
    /**
     * @brief Parse a single line from FTP listing
     * 
     * @param line Single line from listing
     * @return Parsed item string, or empty string if invalid
     */
    static QString parseLine(const QString &line);
};

#endif // FTP_LISTING_PARSER_H

