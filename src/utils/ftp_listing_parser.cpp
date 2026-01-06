#include "ftp_listing_parser.h"
#include <QRegExp>
#include <QStringList>

QStringList FTPListingParser::parseListing(const QString &listing)
{
    QStringList items;

    if (listing.isEmpty()) {
        return items;
    }

    QStringList lines = listing.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        QString parsedItem = parseLine(trimmed);
        if (!parsedItem.isEmpty()) {
            items.append(parsedItem);
        }
    }

    return items;
}

QStringList FTPListingParser::parseListing(const std::string &listing)
{
    return parseListing(QString::fromStdString(listing));
}

QString FTPListingParser::parseLine(const QString &line)
{
    // Parse Unix-style directory listing
    // Format: drwxr-xr-x 2 user group 4096 Jan 1 12:00 filename
    // Or:     -rw-r--r-- 1 user group 1024 Jan 1 12:00 filename
    
    QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);

    // Need at least 9 parts: permissions, links, user, group, size, month, day, time, filename
    if (parts.size() < 9) {
        return QString();
    }

    QString permissions = parts[0];
    bool isDir = permissions.startsWith('d');

    // Filename might contain spaces, so take everything from index 8 onwards
    QString filename = parts.mid(8).join(" ");
    
    // Size is at index 4
    QString size = isDir ? "DIR" : parts[4];
    
    // Date components: month (5), day (6), time (7)
    QString date = parts[5] + " " + parts[6] + " " + parts[7];

    // Return format: "filename\tsize\tdate"
    return filename + "\t" + size + "\t" + date;
}

